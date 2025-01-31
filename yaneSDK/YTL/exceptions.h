// YTLExceptions.h
// Created by derplayer
// Created on 2025-01-26 23:11:39

#ifndef __YTLExceptions_h__
#define __YTLExceptions_h__

#ifdef USE_EXCEPTION

namespace yaneuraoGameSDK3rd {
namespace YTL {

class CException {
/**
 * Base exception class that all other exceptions inherit from
 */
public:
    CException() {
        #ifdef USE_STOP_EXCEPTION
        MessageBox(NULL, "Exception occurred! Process will halt!", "Exception", MB_OK);
        /**
         * After this dialog appears, attach to the process from VS
         * and use the build break to see the call stack and determine
         * where the exception occurred
         */

        // Deliberately cause a memory error for debugging
        * LPLONG(0xcdcdcdcd) = 0;
        // On WinNT/2000 systems, triggering this memory error
        // makes debugging easier
        #endif
    }
    virtual string getError() const { return ""; };
};

class CIndexOutOfBoundsException : public CException {
/**
 * Array bounds violation exception
 * Triggered by smart_ptr check functions when accessing out of bounds
 */
public:
    virtual string getError() const {
        return "Memory access violation: Array index out of bounds";
    }
};

class CNullPointerException : public CException {
/**
 * Null pointer access exception
 * Triggered by smart_ptr check functions when accessing null pointer
 */
public:
    virtual string getError() const {
        return "Memory access violation: Null pointer access";
    }
};

class CRuntimeException : public CException {
/**
 * Runtime exception for invalid value access
 */
public:
    CRuntimeException(const string& strErrorName="") : m_str(strErrorName) {
        if (m_str.empty()) {
            m_str = "Runtime Exception (Invalid value specified)";
        }
    }
    virtual string getError() const {
        return m_str;
    }
protected:
    string m_str;
};

/////////////////////////////////////////////////////////////////
// Thread Exceptions

class CInterruptedException : public CException {
/**
 * Exception thrown when a waiting/sleeping thread receives an interrupt
 * (Similar to Java's InterruptedException)
 */
public:
    virtual string getError() const {
        return "InterruptedException occurred";
    }
};

class CIllegalMonitorStateException : public CException {
/**
 * Exception thrown when a thread calls wait/notify/notifyAll without
 * owning the lock (Similar to Java's IllegalMonitorStateException)
 */
public:
    virtual string getError() const {
        return "IllegalMonitorStateException occurred";
    }
};

/////////////////////////////////////////////////////////////////

class CSyntaxException : public CException {
/**
 * General syntax error exception
 */
public:
    CSyntaxException(const string& strErrorName) : m_str(strErrorName) {}
    virtual string getError() const {
        return m_str;
    }
protected:
    string m_str;
};

} // end of namespace YTL
} // end of namespace yaneuraoGameSDK3rd

#endif  // USE_EXCEPTION
#endif  // __YTLExceptions_h__