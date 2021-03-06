#include "cmdlist.h"

using std::ostream;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::unique_ptr;

void printHeader(ostream& ost, Table const& table, vector<Column> const& cols) {
   auto col = cols.begin();
   ost << col->name();
   for(++col; col != cols.end(); ++col)
      ost << ", " << col->name();
}

Page* getPage(Database const* db, Table const& table, rowcount_t row, pagesize_t& pageOffset) {
   return table.getPage(row,pageOffset);
}

bool printRow(Database const* db, ostream& ost, Table const& table, rowcount_t row,
      vector<Column> const& cols) {
   pagesize_t pageOffset;
   // TODO: This search on every row is unacceptable
   unique_ptr<Page> page(getPage(db, table, row, pageOffset));
   uint8_t res = page->at<uint8_t>(pageOffset);
   if(res != Table::ADD_FLAG) return false;
   auto col = cols.begin();
   pagesize_t offset = pageOffset + col->offset();
   col->type().toString(ost, *page, offset);
   for(++col; col != cols.end(); ++col) {
      offset = pageOffset + col->offset();
      ost << ", ";
      col->type().toString(ost, *page, offset);
   }
   return true;
}

bool checkRow(Database const* db, Table const& table, rowcount_t row, Constraints const& constrs) {
   pagesize_t pageOffset;
   // TODO: I would argue that this is also unacceptable
   unique_ptr<Page> page(getPage(db, table, row, pageOffset));
   uint8_t res = page->at<uint8_t>(pageOffset);
   if(res != Table::ADD_FLAG) { return false; }
   pagesize_t offset = pageOffset;
   for(auto it = constrs.begin(); it != constrs.end(); ++it) {
      pageOffset = offset + it->first.offset();
      if(!it->first.type().satisfies(it->second, *page, pageOffset)) return false;
   }
   return true;
}

bool checkIndex(std::shared_ptr<Index const> index, Constraints const& constrs,
      Constraints& usedConstraints) {
   auto colIt = index->cols().begin();
   for(; colIt != index->cols().end(); ++colIt) {
      auto preds = constrs.find(colIt->first);
      if(preds == constrs.end()) break;
      if((preds->second)[0].op != Predicate::EQ) break;
      usedConstraints[colIt->first].push_back(preds->second[0]);
   }
   if(colIt == index->cols().end()) return true;
   if(index->type() == Index::Hash) return false;
   usedConstraints.clear();
   // TODO: Figure out what constraint combination can be fed to this btree
   return false;
}

// After optimisation each column has constraints
// as either one EQ or (LT|LEQ)?,(GT|GEQ)?,NEQ*
// Returns false if set is contradictive
bool optimizeConstraints(Constraints& constrs) {
   for(auto it = constrs.begin(); it != constrs.end(); ++it) {
      void* lowBound = nullptr;
      bool lowStrict = false;
      void* highBound = nullptr;
      bool highStrict = false;
      std::vector<Predicate> newPred;
      auto& type = it->first.type();
      for(auto pred = it->second.begin(); pred != it->second.end(); ++pred) {
         if(pred->op == Predicate::NEQ) newPred.push_back(*pred); 
         else if(pred->op == Predicate::EQ) {
            if(highBound) {
               int res = type.compare(pred->val, highBound);
               if(res == 1 || (highStrict && !res)) return false;
            }
            if(lowBound) {
               int res = type.compare(pred->val, lowBound);
               if(res == -1 || (lowStrict && !res)) return false;
            }
            lowBound = pred->val;
            highBound = pred->val;
            lowStrict = false;
            highStrict = false;
         } else if(pred->op == Predicate::LEQ) {
            if(highBound) {
               int res = type.compare(pred->val, highBound);
               if(res == -1) {
                  highBound = pred->val;
                  highStrict = false;
               }
            } else {
               highBound = pred->val;
               highStrict = false;
            }
            if(lowBound) {
               int res = type.compare(pred->val, lowBound);
               if(res == -1 || (!res && lowStrict)) return false;
            }
         } else if(pred->op == Predicate::LT) {
            if(highBound) {
               int res = type.compare(pred->val, highBound);
               if(res == -1 || (!highStrict && !res)) {
                  highBound = pred->val;
                  highStrict = true;
               }
            } else {
               highBound = pred->val;
               highStrict = true;
            }
            if(lowBound) {
               int res = type.compare(pred->val, lowBound);
               if(res != 1) return false;
            }
         } else if(pred->op == Predicate::GEQ) {
            if(lowBound) {
               int res = type.compare(pred->val, lowBound);
               if(res == 1) {
                  lowBound = pred->val;
                  lowStrict = false;
               }
            } else {
               lowBound = pred->val;
               lowStrict = false;
            }
            if(highBound) {
               int res = type.compare(pred->val, highBound);
               if(res == 1 || (!res && highStrict)) return false;
            }
         } else if(pred->op == Predicate::GT) {
            if(lowBound) {
               int res = type.compare(pred->val, lowBound);
               if(res == 1 || (!lowStrict && !res)) {
                  lowBound = pred->val;
                  lowStrict = true;
               }
            } else {
               lowBound = pred->val;
               lowStrict = true;
            }
            if(highBound) {
               int res = type.compare(pred->val, highBound);
               if(res != -1) return false;
            }
         }
      }
      if(lowBound && highBound && !lowStrict && !highStrict) {
         int res = type.compare(lowBound, highBound);
         if(!res) {
            for(auto pred = newPred.begin(); pred != newPred.end(); ++pred) {
               int res = type.compare(pred->val, lowBound);
               if(!res) return false;
            }
            newPred.clear();
            newPred.push_back(Predicate(Predicate::EQ, lowBound));
            it->second.swap(newPred);
            return true;
         }
      }
      if(lowBound) {
         Predicate::OPER op = lowStrict ? Predicate::GT : Predicate::GEQ;
         newPred.push_back(Predicate(op, lowBound));
      }
      if(highBound) {
         Predicate::OPER op = highStrict ? Predicate::LT : Predicate::LEQ;
         newPred.push_back(Predicate(op, highBound));
      }
      it->second.swap(newPred);
   }
   return true;
}

// TODO: Really check this shit on dummy(metadata only) implementations of hash and btree.
std::shared_ptr<Index const> findIndex(Database const* db, Table const& table,
      Constraints& constrs, Constraints& usedConstraints, bool& contradiction) {
   if(constrs.empty()) return std::shared_ptr<Index const>();
   contradiction = false;
   if(!optimizeConstraints(constrs)) {
      contradiction = true;
      return std::shared_ptr<Index const>();
   }
   auto indexes = table.indexes(); 
   int best = -1;
   bool isEquality = false;
   for(int i = 0; i != indexes.size(); ++i) {
      auto index = indexes[i];
      Constraints currentConstraints;
      if(best == -1 && checkIndex(index, constrs, currentConstraints)) {
         best = i;
         isEquality = currentConstraints.begin()->second.front().op == Predicate::EQ;
         usedConstraints = currentConstraints;
      } else if(best == -1) continue;
      else {
         auto bestIndex = indexes[best];
         if(index->type() == Index::Hash) {
            if(isEquality && !index->unique() && bestIndex->unique()) continue;
            if(isEquality && (index->cols().size() <= bestIndex->cols().size())) continue;
            if(bestIndex->type() == Index::Hash &&
                  (index->cols().size() == bestIndex->cols().size())) continue;
            if(checkIndex(index, constrs, currentConstraints)) {
               best = i;
               isEquality = currentConstraints.begin()->second.front().op == Predicate::EQ;
               usedConstraints = currentConstraints;
            }
         } else {
            if(bestIndex->type() == Index::Hash) continue;
            if(isEquality && !index->unique() && bestIndex->unique()) continue;
            if(!checkIndex(index, constrs, currentConstraints)) continue;
            bool currentIsEquality = currentConstraints.begin()->second.front().op == Predicate::EQ;
            if(!currentIsEquality && isEquality) continue;
            if(isEquality == currentIsEquality) {
               if(!index->unique() && bestIndex->unique()) continue;
               if(index->unique() == bestIndex->unique() &&
                  index->cols().size() <= bestIndex->cols().size()) continue;
               
            }
            best = i;
            isEquality = currentIsEquality;
            usedConstraints = currentConstraints;
         }
      }
   }
   if(best == -1) return std::shared_ptr<Index>();
   else return indexes[best];
}

void selectWhere(Database const* db, ostream& ost, Table const& table, Constraints const& constrs,
      Columns cols) {
   Constraints usedConstrs;
   Constraints constraints(constrs);
   bool contr = false;
   auto index = findIndex(db, table, constraints, usedConstrs, contr);
   if(contr) {
      std::cerr << "Constraints set is contradictive" << endl;
      return;
   }
   if(cols.empty()) cols = table.cols();
   printHeader(ost, table, cols);
   //if(index) std::cerr << "Select uses index" << std::endl;
   rowiterator rowIt = index ? index->rowIterator(usedConstrs) : table.rowIterator();
   for(; rowIt; ++rowIt) {
      rowcount_t row = *rowIt;
      if(checkRow(db, table, row, constrs)) {
         ost << endl;
         printRow(db, ost, table, row, cols);
      }
   }
}

// TODO: Has high self time. Investigate further. It's possibly already gone.
bool insertInto(Database const* db, Table& table, Values const& vals) {
   rowcount_t rowNum = table.rowCount();
   table.rowCount() += 1;
   pagesize_t pageOffset;
   unique_ptr<Page> page(getPage(db, table, rowNum, pageOffset));
   page->at<uint8_t>(pageOffset, Table::ADD_FLAG);
   for(auto col = table.cols().begin(); col != table.cols().end(); ++col) {
      auto val = vals.find(*col);
      col->type().write(val->second, *page, pageOffset);
   }
   auto indexes = table.indexes();
   for(auto index = indexes.begin(); index != indexes.end(); ++index)
      if(!(*index)->insert(rowNum, vals)) return false;
   return true;
}

rowcount_t updateWhere(Database const* db, Table& table, Constraints const& constrs,
      Values const& vals) {
   Constraints usedConstrs; 
   Constraints constraints(constrs);
   bool contr;
   auto index = findIndex(db, table, constraints, usedConstrs, contr);
   if(contr) {
      std::cerr << "Constraints set is contradictive" << endl;
      return 0;
   }
   //if(index) std::cerr << "Update uses index" << std::endl;
   rowiterator rowIt = index ? index->rowIterator(usedConstrs) : table.rowIterator();
   rowcount_t rowCount = 0;
   for(; rowIt; ++rowIt) {
      rowcount_t row = *rowIt;
      if(checkRow(db, table, row, constrs)) {
         ++rowCount;
         pagesize_t pageOffset;
         unique_ptr<Page> page(getPage(db, table, row, pageOffset));
         std::map<Column, void*> rowVals;
         for(auto col = table.cols().begin(); col != table.cols().end(); ++col) {
            auto val = vals.find(*col);
            if(val != vals.end()) {
               pagesize_t offset = pageOffset + 1 + col->offset();
               col->type().write(val->second, *page, offset);
            }
            pagesize_t offset = pageOffset + 1 + col->offset();
            rowVals[*col] = col->type().read(*page, offset); 
         }
         auto indexes = table.indexes();
         for(auto index = indexes.begin(); index != indexes.end(); ++index) {
            (*index)->remove(table, rowIt);
            (*index)->insert(rowIt.row(), rowVals);
         }
         for(auto it = rowVals.begin(); it != rowVals.end(); ++it) {
            it->first.type().clear(it->second); 
         }
      }
   }
   return rowCount;
}

rowcount_t deleteWhere(Database const* db, Table& table, Constraints const& constrs) {
   Constraints usedConstrs; 
   Constraints constraints(constrs);
   bool contr;
   auto index = findIndex(db, table, constraints, usedConstrs, contr);
   if(contr) {
      std::cerr << "Constraints set is contradictive" << endl;
      return 0;
   }
   rowcount_t rowCount = 0;
   //if(index) std::cerr << "Delete uses index" << std::endl;
   rowiterator rowIt = index ? index->rowIterator(usedConstrs) : table.rowIterator();
   for(; rowIt; ++rowIt) {
      rowcount_t row = *rowIt;
      if(checkRow(db, table, row, constrs)) {
         ++rowCount;
         pagesize_t pageOffset;
         unique_ptr<Page> page(getPage(db, table, row, pageOffset));
         page->at<uint8_t>(pageOffset, Table::DEL_FLAG);
         auto indexes = table.indexes();
         for(auto index = indexes.begin(); index != indexes.end(); ++index)
            (*index)->remove(table, rowIt);
      }
   }
   return rowCount;
}

void createIndex(Database const* db, Table& table, Index::Type type, bool isUnique, IndexColumns const& cols) {
   auto index = Index::createIndex(db, table, type, isUnique, cols); 
   table.addIndex(index);
   auto rowIt = table.rowIterator();
   for(; rowIt; ++rowIt) {
      pagesize_t offset;
      std::shared_ptr<Page> page(table.getPage(*rowIt, offset));
      if(page->at<uint8_t>(offset) != 0xaa) continue;
      std::map<Column, void*> vals;
      for(auto it = table.cols().begin(); it != table.cols().end(); ++it) {
         vals[*it] = it->type().read(*page, offset); 
      }
      index->insert(*rowIt, vals);
      for(auto it = vals.begin(); it != vals.end(); ++it) {
         it->first.type().clear(it->second);
      }
   }
}
