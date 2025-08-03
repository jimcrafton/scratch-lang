
#pragma once

#ifndef _STRING_REF_H__
#define _STRING_REF_H__

#include <string>

template<typename chT>
class basic_string_ref {
public:
    typedef chT CharT;
protected:
    const CharT* strPtr;    
    size_t len;
public:    
    
    typedef std::char_traits<CharT> char_traits;
    typedef std::basic_string<CharT> StringT;
    typedef typename StringT::const_iterator  const_iterator;

    enum {
        npos = StringT::npos
    };

    basic_string_ref() :strPtr(NULL), len(0) {}
    basic_string_ref(const CharT* b, size_t l) :strPtr(b), len(l) {}

    basic_string_ref& assign(const CharT* b, size_t l) {
        strPtr = b;
        len = l;
        return *this;
    }

    const CharT* data() const {
        if (empty()) {
            return NULL;
        }

        return strPtr;
    }

    const StringT str() const {
        if (empty()) {
            return StringT();
        }

        return StringT(strPtr, len);
    }

    size_t size() const {
        return len;
    }

    size_t length() const { return len; }

    bool empty() const { return len == 0; }
        

    const_iterator begin() const {
        return strPtr;
    }

    const_iterator end() const {
        return &strPtr[len];
    }

    void clear() {
        strPtr = NULL;
        len = 0;
    }

    bool isEqual(const CharT* buffer, size_t bufferLen, bool caseSensitive=false) const {
        if (strPtr == buffer && bufferLen == len) {
            return true;
        }
        if (0 == len && 0 == bufferLen) {
            return true;
        }

        if (len > 0 && bufferLen > 0) {
            return char_traits::compare(strPtr, buffer, len< bufferLen?len: bufferLen) == 0;
        }

        return false;
    }

    
    const CharT& front() const {
        return strPtr[0];
    }

    const CharT& back() const {
        return strPtr[len-1];
    }

    const CharT& operator[](const size_t& pos) const {
        return strPtr[pos];
    }

    bool operator==(const basic_string_ref<CharT>& rhs) const {
        return isEqual(rhs.strPtr, rhs.len);
    }

    bool operator!=(const basic_string_ref<CharT>& rhs) const {
        return !isEqual(rhs.strPtr, rhs.len);
    }

    bool operator==(const std::basic_string<CharT>& rhs) const {
        return isEqual(rhs.c_str(), rhs.length());
    }

    bool operator!=(const std::basic_string<CharT>& rhs) const {
        return !isEqual(rhs.c_str(), rhs.length());
    }

    bool operator==(const CharT* rhs) const {
        return isEqual(rhs, strlen(rhs));
    }

    bool operator!=(const CharT* rhs) const {
        return !isEqual(rhs, strlen(rhs));
    }

    bool operator>(const basic_string_ref<CharT>& rhs) const {
        return char_traits::compare(strPtr, rhs.strPtr, len < rhs.len ? len : rhs.len) > 0;
    }

    bool operator>(const std::basic_string<CharT>& rhs) const {
        return char_traits::compare(strPtr, rhs.c_str(), len < rhs.length() ? len : rhs.length()) > 0;
    }

    bool operator>(const CharT* rhs) const {
        size_t rhslen = strlen(rhs);
        return char_traits::compare(strPtr, rhs, len < rhslen ? len : rhslen) > 0;
    }

    bool operator<(const basic_string_ref<CharT>& rhs) const {
        return char_traits::compare(strPtr, rhs.strPtr, len < rhs.len ? len : rhs.len) < 0;
    }

    bool operator<(const std::basic_string<CharT>& rhs) const {
        return char_traits::compare(strPtr, rhs.c_str(), len < rhs.length() ? len : rhs.length()) < 0;
    }

    bool operator<(const CharT* rhs) const {
        size_t rhslen = strlen(rhs);
        return char_traits::compare(strPtr, rhs, len < rhslen ? len : rhslen) < 0;
    }

};


typedef basic_string_ref<char> string_ref; 


#endif //_STRING_REF_H__