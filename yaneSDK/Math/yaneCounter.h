//
// Game-purpose Generic Counter
//

#ifndef __yaneCounter_h__
#define __yaneCounter_h__

#include "../Auxiliary/yaneSerialize.h"

namespace yaneuraoGameSDK3rd {
namespace Math {

class ICounter;

class ICounterFactory {
/*
    When users prepare their own counter (ICounter-derived class),
    GetType should return a unique number greater than 1000,
    Override CreateInstance in this Factory-derived class,
    When receiving that number, return a new instance of that class,
    Then set that Factory using ICounter::SetFactory.

    This will enable serialization of that newly created counter
    using CProxyCounter.
*/
public:
    virtual ICounter* CreateInstance(int n)=0;
    virtual ~ICounterFactory(){}
};

class ICounter : public IArchive {
/*
    Base class for counters
*/
public:
    /// Compatibility with int type
    virtual operator int () const=0;
    virtual const int operator = (int n)=0;

    /// Increment/Decrement operations
    virtual void    Inc()=0;
    virtual void    Dec()=0;

    /// Is it at initial position?
    virtual bool IsBegin() const = 0;
    /// Has it reached the end?
    virtual bool IsEnd() const = 0;

    virtual int GetType() const = 0;
    static    ICounter* CreateInstance(int nType);
    /*
        Manual RTTI
            CNullCounter        : 0
            CRootCounter       : 1
            CSaturationCounter : 2
            CInteriorCounter   : 3
        GetType returns the dynamic type number
        CreateInstance is a factory to create that object from the number
        If the number is other than above, it uses the factory set by SetFactory to create
    */
    static    void    SetFactory(const smart_ptr<ICounterFactory>& v)
        {    m_vFactory = v; }
    static    smart_ptr<ICounterFactory>    GetFactory()
        { return m_vFactory; }

    virtual ~ICounter(){}

protected:
    static    smart_ptr<ICounterFactory> m_vFactory;
};

class CNullCounter : public ICounter {
/*
    Null Device for class ICounter
*/
public:
    virtual operator int () const { return 0; }
    virtual const int operator = (int n) { return n; }
    virtual void    Inc() {}
    virtual void    Dec() {}
    virtual bool IsBegin() const { return false;}
    virtual bool IsEnd() const { return false; }
    virtual int GetType() const { return 0; }
    virtual void Serialize(ISerialize&) {}
};

/////////////////////////////////////////////////////////////////////////

class CProxyCounter : public ICounter {
/*
    Proxy class for ICounter. This class uses ICounter's factory
    to recreate the object during deserialization.
*/
public:
    CProxyCounter() : m_vCounter(new CNullCounter) {}
    CProxyCounter(const smart_ptr<ICounter>& p) : m_vCounter(p){}

    /// Compatibility with int type
    virtual operator int () const { return *m_vCounter.get(); }
    virtual const int operator = (int n) { return ((*m_vCounter.get()) = n);}

    /// Increment/Decrement operations
    virtual void    Inc() { m_vCounter->Inc(); }
    virtual void    Dec() { m_vCounter->Dec(); }

    /// Is it at initial position?
    virtual bool IsBegin() const { return m_vCounter->IsBegin(); }
    /// Has it reached the end?
    virtual bool IsEnd() const { return m_vCounter->IsEnd(); }

protected:
    smart_ptr<ICounter>    m_vCounter;
    ICounter* GetCounter() { return m_vCounter.get(); }

    // overridden from IArchive
    virtual void Serialize(ISerialize&);
};

/////////////////////////////////////////////////////////////////////////

class CRootCounter : public ICounter {
/*
    Sets initial value (nStart), end value (nEnd), and if needed, increment value (nStep).
    After that, executing Inc function will increment by 1 (or by nStep if set).
    When the result of addition becomes >= nEnd, it automatically returns to nStart.
    (It doesn't become nEnd)

    Reset function returns counter value to initial value nStart.
    Or if an initial value is set by SetInit function after Reset, it will be set to that value.

    Also, since it's mutually convertible with int type, it can be used as if it were an int variable.

    Example:
    CRootCounter r;
    r.Set(0, 256, -5);
    // Sets counter with nStart == 0, nEnd == 256
    r = 128;

    In this state, executing r++; 5 times will make r == 129 on the 5th time.
    Executing r--; 5 times will make r == 127 on the 5th time.

    Also, it doesn't have to be nStart...nEnd.
    That is, addition (Inc / ++) increments toward nEnd direction.
    Subtraction (Dec / --) increments toward nStart direction.

    Also, when Step is negative,
    it advances 1 step toward nEnd direction for absolute value times of Inc member function calls.
*/
public:
    /// nStep is the absolute value of increment per step. Negative means 1/nStep
    /// Can be nStart...nEnd or not
    void    Set(int nStart,int nEnd,int nStep=1)
        { m_nStart=nStart; m_nEnd=nEnd; m_nStep=nStep; Reset(); }
    void    SetStep(int nStep) { m_nStep = nStep; }
    void    SetStart(int nStart) { m_nStart = nStart; }
    void    SetEnd(int nEnd) { m_nEnd = nEnd; }

    /// Getters
    int        GetStep() const { return m_nStep; }
    int        GetStart() const { return m_nStart; }
    int        GetEnd() const { return m_nEnd; }

    /// Reset counter
    void    Reset() { m_nRootCount= m_nStart; m_nRate=0; }

    /// property..
    virtual bool    IsBegin() const { return m_nRootCount == m_nStart; }
    virtual bool    IsEnd() const { return m_nRootCount == m_nEnd; }

    CRootCounter();
    CRootCounter(int nEnd);
    CRootCounter(int nStart,int nEnd,int nStep=1);

    // Mutual conversion with int type
    operator int () const { return m_nRootCount; }
    const int operator = (int n) { m_nRootCount = n; return n; }
    int        Get () const { return m_nRootCount; }

    // Counter increment (stops when reaching the end)
    void    Inc() { inc(true); }
    void    Dec() { inc(false); }
    // Addition (increment toward End direction) / Subtraction (increment toward Start direction)
    CRootCounter& operator++()
        { Inc(); return (*this); }
    CRootCounter operator++(int)
        { CRootCounter _Tmp = *this; Inc(); return (_Tmp); }
    CRootCounter& operator--()
        { Dec(); return (*this); }
    CRootCounter operator--(int)
        { CRootCounter _Tmp = *this; Dec(); return (_Tmp); }

    virtual int GetType() const { return 1; }

protected:
    void    inc(bool bAdd=true);

    int        m_nRootCount;
    int        m_nStart;
    int        m_nEnd;
    int        m_nStep;
    int        m_nRate;    // When nStep<0, incremented by +1 on first Inc() call

    virtual void Serialize(ISerialize&s) {
        s << m_nRootCount << m_nStart << m_nEnd << m_nStep << m_nRate;
    }
};

class CSaturationCounter : public ICounter {
/*
    Saturation Counter. A variation of class CRootCounter.
    The only difference is "when the counter is incremented and reaches the end value,
    it stops at that point"
*/
public:
    /// nStep is the absolute value of increment per step. Negative means 1/nStep
    /// Can be nStart...nEnd or not
    void    Set(int nStart,int nEnd,int nStep=1)
        { m_nStart=nStart; m_nEnd=nEnd; m_nStep=nStep; Reset(); }
    void    SetStep(int nStep) { m_nStep = nStep; }
    void    SetStart(int nStart) { m_nStart = nStart; }
    void    SetEnd(int nEnd) { m_nEnd = nEnd; }

    /// Getters
    int        GetStep() const { return m_nStep; }
    int        GetStart() const { return m_nStart; }
    int        GetEnd() const { return m_nEnd; }

    /// Reset counter
    void    Reset() { m_nRootCount= m_nStart; m_nRate=0; }

    /// property..
    virtual bool    IsBegin() const { return m_nRootCount == m_nStart; }
    virtual bool    IsEnd() const { return m_nRootCount == m_nEnd; }

    CSaturationCounter();
    CSaturationCounter(int nEnd);
    CSaturationCounter(int nStart,int nEnd,int nStep=1);

    // Mutual conversion with int type
    operator int () const { return m_nRootCount; }
    const int operator = (int n) { m_nRootCount = n; return n; }
    int        Get () const { return m_nRootCount; }

    // Counter increment (stops when reaching the end)
    void    Inc() { inc(true); }
    void    Dec() { inc(false); }
    // Addition (increment toward End direction) / Subtraction (increment toward Start direction)
    CSaturationCounter& operator++()
        { Inc(); return (*this); }
    CSaturationCounter operator++(int)
        { CSaturationCounter _Tmp = *this; Inc(); return (_Tmp); }
    CSaturationCounter& operator--()
        { Dec(); return (*this); }
    CSaturationCounter operator--(int)
        { CSaturationCounter _Tmp = *this; Dec(); return (_Tmp); }

    virtual int GetType() const { return 2; }

protected:
    void    inc(bool bAdd=true);

    int        m_nRootCount;
    int        m_nStart;
    int        m_nEnd;
    int        m_nStep;
    int        m_nRate;    // When nStep<0, incremented by +1 on first Inc() call

    virtual void Serialize(ISerialize&s) {
        s << m_nRootCount << m_nStart << m_nEnd << m_nStep << m_nRate;
    }
};

class CInteriorCounter : public ICounter {
/*
    Provides an interpolation counter.

    Set the initial value (nStart), end value (nEnd),
    and division count (nFrames) using the Set member function.

    This counter will then start from nStart and reach nEnd
    after calling Inc member function nFrames times.

    Inc is also defined as operator++.
    Dec, which is the reverse operation of Inc, also exists and is defined as operator--.
*/
public:
    CInteriorCounter();

    /// Mutual conversion with int type
    virtual operator int () const { return m_nNow; }
    virtual const int operator = (int n) { m_nNow = m_nStart = n; m_nFramesNow = 0; return n; }

    virtual void    Inc();        /// Addition
    CInteriorCounter& operator++() { Inc(); return (*this); }
    CInteriorCounter operator++(int) { CInteriorCounter _Tmp = *this; Inc(); return (_Tmp); }

    virtual void    Dec();        /// Subtraction
    CInteriorCounter& operator--() { Dec(); return (*this); }
    CInteriorCounter operator--(int) { CInteriorCounter _Tmp = *this; Dec(); return (_Tmp); }

    void    Set(int nStart,int nEnd,int nFrames);
    /*
        Sets initial value (nStart), end value (nEnd), and division count (nFrames)
    */

    /// Temporarily changes current value. Returns to normal value on next Inc/Dec
    void    Set(int nNow) { *this = nNow; }

    virtual bool    IsBegin() const { return (m_nNow == m_nStart);}
    virtual bool    IsEnd() const { return (m_nNow == m_nEnd);}

    /// property
    int        GetFrame() const { return m_nFrames; }
    int        GetStart() const { return m_nStart;}
    int        GetEnd() const { return m_nEnd;}
    /// Get current frame count (number of times inc was called)
    int        GetFrameNow() const { return m_nFramesNow; }

    virtual int GetType() const { return 3; }

protected:
    int        m_nNow;            // Current value
    int        m_nStart;        // Initial value
    int        m_nEnd;            // End value
    int        m_nFrames;        // Frame division count (how many Inc calls until end value)
    int        m_nFramesNow;    // Current frame number

    virtual void Serialize(ISerialize&s) {
        s << m_nNow << m_nStart << m_nEnd << m_nFrames << m_nFramesNow;
    }
};

/////////////////////////////////////////////////////////////////////////

} // end of namespace Math
} // end of namespace yaneuraoGameSDK3rd

#endif