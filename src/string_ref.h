
#ifndef _STRING_REF_H__
#define _STRING_REF_H__


template<typename chT>
class basic_string_ref {
public:
    typedef chT CharT;
protected:
    const CharT* buffer;    
    size_t len;
public:
    typedef chT CharT;
    typedef std::basic_string<CharT> StringT;
    typedef typename StringT::const_iterator  const_iterator;

    enum {
        npos = StringT::npos
    };

    basic_string_ref() :buffer(NULL), len(0) {}
    basic_string_ref(const CharT* b, size_t l) :buffer(b), len(l) {}

    basic_string_ref& assign(const CharT* b, size_t l) {
        buffer = b;
        len = l;
        return *this;
    }

    const CharT* data() const {
        if (empty()) {
            return NULL;
        }

        return buffer;
    }

    const StringT str() const {
        if (empty()) {
            return StringT();
        }

        return StringT(buffer, len);
    }

    size_t size() const {
        return len;
    }

    size_t length() const { return len; }

    bool empty() const { return len == 0; }
        

    const_iterator begin() const {
        return buffer;
    }

    const_iterator end() const {
        return &buffer[len];
    }

    void clear() {
        buffer = NULL;
        len = 0;
    }
};


typedef basic_string_ref<char> string_ref; 


#endif //_STRING_REF_H__