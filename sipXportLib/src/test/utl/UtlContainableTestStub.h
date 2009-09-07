#ifndef _UtlContainableTestStub_h
#define _UtlContainableTestStub_h

#include "utl/UtlContainable.h"

class UtlContainableTestStub : public UtlContainable
{
private:
        int id ;
        static int cCount ;
        static const UtlContainableType TYPE;
        // Nobody should ever be calling the no arg constructor
        UtlContainableTestStub() ;

public :
    UtlContainableTestStub(int ) ;
    virtual ~UtlContainableTestStub() ;
    static void printCount() ;
    virtual UtlBoolean isEqual(const UtlContainable* c) const ;
    virtual unsigned hash() const ;
    virtual int compareTo(UtlContainable const* c) const ;
    virtual UtlContainableType getContainableType() const;
    virtual UtlBoolean isInstance(UtlContainable const*) const ;
    static int getCount() ;
    static void clearCount() ;
}  ;

#endif
