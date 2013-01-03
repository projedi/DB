bool File::operator ==(File const& o) const { return m_name == o.m_name; }
bool File::operator <(File const& o) const { return m_name < o.m_name; }

std::string const& File::name() const { return m_name; }
