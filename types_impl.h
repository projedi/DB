template<class T>
Predicate::Predicate(OPER op, T* val): op(op), val((void*)val) { }

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
void IntType::write(void* val, Page& page, pagesize_t& offset) const {
   page.at<int32_t>(offset, *((int32_t*)val));
}
void* IntType::read(Page const& page, pagesize_t& offset) const { 
   return new int32_t(page.at<int32_t>(offset));
}
void IntType::clear(void* p) const { delete (int32_t*)p; }
void IntType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<int32_t>(offset); 
}
bool IntType::satisfies(std::vector<Predicate> const& pred, Page const& page, pagesize_t& off) const {
   return satisfiesPage<int32_t>(pred, page, off);
}
uint32_t IntType::hash(void* val, pagesize_t size) const {
   return std::hash<int32_t>()(*((int32_t*)val)) % size;
}
int IntType::compare(void* lhs, void* rhs) const {
   int32_t lval = *(int32_t*)lhs;
   int32_t rval = *(int32_t*)rhs;
   if(lval < rval) return -1;
   if(lval == rval) return 0;
   return 1;
}

typesize_t DoubleType::size() const { return sizeof(double); }
typeid_t DoubleType::id() const { return 2; }
void DoubleType::write(void* val, Page& page, pagesize_t& offset) const {
   page.at<double>(offset, *((double*)val));
}
void* DoubleType::read(Page const& page, pagesize_t& offset) const { 
   return new double(page.at<double>(offset));
}
void DoubleType::clear(void* p) const { delete (double*)p; }
void DoubleType::toString(std::ostream& ost, Page const& page, pagesize_t& offset) const {
   ost << page.at<double>(offset); 
}
bool DoubleType::satisfies(std::vector<Predicate> const& pred, Page const& page, pagesize_t& off) const {
   return satisfiesPage<double>(pred, page, off);
}
uint32_t DoubleType::hash(void* val, pagesize_t size) const {
   return std::hash<double>()(*((double*)val)) % size;
}
int DoubleType::compare(void* lhs, void* rhs) const {
   double lval = *(double*)lhs;
   double rval = *(double*)rhs;
   if(lval < rval) return -1;
   if(lval == rval) return 0;
   return 1;
}

VarcharType::VarcharType(typesize_t size): m_size(size) { }
typesize_t VarcharType::size() const { return m_size; }
// By specification string can't be longer than 128
typeid_t VarcharType::id() const { return 3 + 256 * m_size; }
void VarcharType::write(void* val, Page& page, pagesize_t& offset) const {
   char* str = (char*)val;
   for(typesize_t i = 0; i != m_size; ++i) {
      if(*str) { page.at<char>(offset, *str); ++str; } 
      else page.at<char>(offset, 0);
   }
}
void* VarcharType::read(Page const& page, pagesize_t& offset) const { 
   char* res = new char[m_size + 1];
   char* str = res;
   for(typesize_t i = 0; i != m_size; ++i, ++str) {
      *str = page.at<char>(offset);
   }
   *str = 0;
   return res;
}
void VarcharType::clear(void* p) const { delete[] (char*)p; }
bool VarcharType::satisfies(std::vector<Predicate> const& preds, Page const& page, pagesize_t& off) const {
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
uint32_t VarcharType::hash(void* val, pagesize_t size) const {
   char* c = (char*)val;
   std::hash<char> hasher;
   size_t res = hasher(*c);
   ++c;
   for(;*c;++c) res ^= hasher(*c);
   return res % size;
}
int VarcharType::compare(void* lhs, void* rhs) const {
   return strcmp((char*)lhs, (char*)rhs);
}

SqlType* identifyType(typeid_t id) {
   if(id == 1) return new IntType();
   if(id == 2) return new DoubleType();
   if(!((id - 3) % 256)) return new VarcharType((id - 3)/256);
   return nullptr;
}
