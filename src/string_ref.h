
#ifndef _STRING_REF_H__
#define _STRING_REF_H__


template<typename chT>
class basic_string_ref {
public:
    typedef chT CharT;
protected:
    const CharT* buffer;
    size_t pos;
    size_t len;
public:
    typedef chT CharT;
    typedef std::basic_string<CharT> StringT;
    typedef typename StringT::const_iterator  const_iterator;

    enum {
        npos = StringT::npos
    };

    basic_string_ref() :buffer(NULL), pos(0), len(0) {}
    basic_string_ref(const CharT* b, size_t p, size_t l) :buffer(b), pos(p), len(l) {}

    basic_string_ref& assign(const CharT* b, size_t p, size_t l) {
        buffer = b;
        pos = p;
        len = l;
        return *this;
    }

    const CharT* data() const {
        return buffer;
    }

    const StringT str() const {
        return StringT(&buffer[pos], len);
    }

    size_t size() const {
        return len;
    }
    size_t length() const { return len; }

    bool empty() const { len == 0; }

    size_t offset() const {
        return pos;
    }

    const_iterator begin() const {
        return &buffer[pos];
    }

    const_iterator end() const {
        return &buffer[pos + len];
    }
};


typedef basic_string_ref<char> string_ref; 


#endif //_STRING_REF_H__