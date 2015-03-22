/**********************************************************
Copyright 2014 by Luca Zini and Francesca Odone,
Department of Informatics, Bioengineering, Robotics, and Systems
Engineering University of Genova.

This is a free software: you can redistribute it and/or modify it
under the terms of the CC-BY Public License, Version 4.0, 25
November 2013. This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY, including but not limited to
the warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the CC-BY Public License copy provided with this software for more
details.

@author Luca Zini (luca.zini@gmail.com)

***********************************************************/
#ifndef DEBUG_STREAM_H
#define DEBUG_STREAM_H

#include <iostream>

class DebugStream{
public:


    static DebugStream& getInstance(){
        static DebugStream instance;
        return instance;
    }

    void setStream(std::ostream* stream){
        this->stream = stream;
    }


    template <class T >
    DebugStream& operator << (T data    ){
        if(stream != 0){
            (*stream) << data;

        stream->flush();
        stream->clear();
        }

        return *this;

    }

private:
    DebugStream():stream(0){ }

    std::ostream* stream;
};

static DebugStream& dstream = DebugStream::getInstance();

#endif // DEBUG_STREAM_H
