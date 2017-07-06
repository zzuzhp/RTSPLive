#ifndef ___AACSTREAM_H___
#define ___AACSTREAM_H___

#include "AVStream.h"
#include "XUtil/XURefCount.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class AACStream : public AudioStream,
                  public XURefCount
{
public:

    static AACStream * create_instance(uint32_t id);

    unsigned long addRef()
    {
        unsigned long ref = XURefCount::addRef();
        return ref;
    }

    unsigned long release()
    {
        unsigned long ref = XURefCount::release();
        return ref;
    }

    void push_frame(char * data, int len, uint32_t ts) {}

private:

    AACStream(uint32_t id);

    ~AACStream();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___AACSTREAM_H___
