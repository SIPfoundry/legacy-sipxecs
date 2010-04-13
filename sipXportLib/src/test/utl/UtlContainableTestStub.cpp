#include <utl/UtlContainableTestStub.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int UtlContainableTestStub::cCount = 0 ;

const UtlContainableType UtlContainableTestStub::TYPE = "UtlContainableTestStub" ;
UtlContainableTestStub::UtlContainableTestStub()
{
    cCount++ ;
    id = -1 ;
}

UtlContainableTestStub::UtlContainableTestStub(int i)
{
    id = i ;
    cCount++ ;
}

UtlContainableTestStub :: ~UtlContainableTestStub()
{
    UtlContainableTestStub::cCount-- ;
}

void UtlContainableTestStub :: printCount()
{
    printf("Creation count = %d;\n", cCount) ;
}

UtlBoolean UtlContainableTestStub :: isEqual(const UtlContainable* c) const
{
    return (compareTo(c) == 0) ;
}

unsigned UtlContainableTestStub :: hash() const
{
    return id ;
}

int UtlContainableTestStub :: compareTo(UtlContainable const* c) const
{
    int cmp = 1 ;
    if (c->getContainableType() == TYPE)
    {
        UtlContainableTestStub* t = (UtlContainableTestStub*)c ;
        int inId = t -> id ;
        cmp = id - inId ;
    }
    return cmp ;
}

UtlContainableType UtlContainableTestStub::getContainableType() const
{
    return TYPE;
}

int UtlContainableTestStub :: getCount()
{
    return cCount ;
}

void UtlContainableTestStub::clearCount()
{
    cCount = 0 ;
}

UtlBoolean UtlContainableTestStub::isInstance(UtlContainable const* c) const
{
    bool result = false ;
    UtlContainableType cType = c -> getContainableType() ;
    result = (strcmp(cType, "UtlContainableTestStub") == 0) ;
    return result;
}
