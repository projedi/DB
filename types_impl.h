Predicate::Predicate(OPER op, uint8_t const* val): op(op), val(val) { }

template<class T>
bool satisfiesVal(T const& lval, Predicate::OPER op, T const& rval) {
   switch(op) {
      case Predicate::EQ: return lval == rval;
      case Predicate::NEQ: return lval != rval;
      case Predicate::LT: return lval < rval;
      case Predicate::GT: return lval > rval;
      case Predicate::LEQ: return lval <= rval;
      case Predicate::GEQ: return lval >= rval;
   }
}

template<class T>
bool satisfiesPage(std::vector<Predicate> const& preds, Page const& page, pagesize_t off) {
   T pval = page.at<T>(off);
   for(auto pred = preds.begin(); pred != preds.end(); ++pred) {
      T val = *((T*)pred->val);
      if(!satisfiesVal(pval, pred->op, val)) return false;
   }
   return true;
}

typesize_t IntType::size() const { return sizeof(int32_t); }
typeid_t IntType::id() const { return 1; }
void IntType::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   int32_t res;
   ist >> res;
   page.at<int32_t>(offset, res);
}
void IntType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<int32_t>(offset); 
}
bool IntType::satisfies(std::vector<Predicate> const& pred, Page const& page, pagesize_t off) const {
   return satisfiesPage<int32_t>(pred, page, off);
}

typesize_t DoubleType::size() const { return sizeof(double); }
typeid_t DoubleType::id() const { return 2; }
void DoubleType::fromString(std::istream& ist, Page& page, pagesize_t& offset) const {
   double res;
   ist >> res;
   page.at<double>(offset, res);
}
void DoubleType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<double>(offset); 
}
bool DoubleType::satisfies(std::vector<Predicate> const& pred, Page const& page, pagesize_t off) const {
   return satisfiesPage<double>(pred, page, off);
}

VarcharType::VarcharType(typesize_t size): m_size(size) { }
typesize_t VarcharType::size() const { return m_size; }
// By specification string can't be longer than 128
typeid_t VarcharType::id() const { return 3 + 256 * m_size; }
bool VarcharType::satisfies(std::vector<Predicate> const& preds, Page const& page, pagesize_t off) const {
   char* pval = new char[m_size + 1];
   std::unique_ptr<char> holder(pval);
   for(int i = 0; i != m_size; ++i, ++pval)
      *pval = page.at<char>(off);
   *pval = 0;
   pval = holder.get();
   for(auto pred = preds.begin(); pred != preds.end(); ++pred) {
      char* val = (char*)pred->val;
      int res = strcmp(pval, val);
      switch(pred->op) {
         case Predicate::EQ: if(res) return false; break;
         case Predicate::NEQ: if(!res) return false; break;
         case Predicate::LT: if(res >= 0) return false; break;
         case Predicate::GT: if(res <= 0) return false; break;
         case Predicate::LEQ: if(res > 0) return false; break;
         case Predicate::GEQ: if(res < 0) return false; break;
      }
   }
   return true;
}

SqlType* identifyType(typeid_t id) {
   if(id == 1) return new IntType();
   if(id == 2) return new DoubleType();
   if(!((id - 3) % 256)) return new VarcharType((id - 3)/256);
   return nullptr;
}
