#ifndef ___XULOG_H___
#define ___XULOG_H___

#include "XUMutex.h"
#include "XUTime.h"

#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>

#if defined(WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#include <iostream>
#elif defined(ANDROID)
#include <android/log.h>
#endif

#define TAG "XULOG"

/////////////////////////////////////////////////////////////////////////////
////

#define __FILENAME__    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define WHERE_AM_I      WhereAMI(__FUNCTION__, __FILENAME__, __LINE__).where()

struct WhereAMI
{
    WhereAMI()
    {

    }

    WhereAMI(const std::string & function,
             const std::string & file,
             const size_t      & line) : m_function(function), m_file(file), m_line(line)
    {

    }

    const std::string & file() const { return m_file; }
    const std::string & function() const { return m_function; }
    size_t line_number() const { return m_line; }

    const std::string where() const
    {
        std::ostringstream oss;
        oss << m_file << "[" << m_function << "(" << m_line << ")" << "]";

        return oss.str();
    }

private:

    std::string     m_function;
    std::string     m_file;
    size_t          m_line;
};

typedef enum
{
    LOG_LEVEL_VERBOSE   = 0,
    LOG_LEVEL_DEBUG     = 1,
    LOG_LEVEL_INFO      = 2,
    LOG_LEVEL_WARNING   = 3,
    LOG_LEVEL_ERROR     = 4,
    LOG_LEVEL_FATAL     = 5
} XULOGLevel;

static
std::ostream & operator <<(std::ostream & lhs, const XULOGLevel & rhs)
{
    switch (rhs)
    {
        case LOG_LEVEL_VERBOSE: lhs << "[verbose]"; break;
        case LOG_LEVEL_DEBUG:   lhs << "[debug  ]"; break;
        case LOG_LEVEL_INFO:    lhs << "[info   ]"; break;
        case LOG_LEVEL_WARNING: lhs << "[warning]"; break;
        case LOG_LEVEL_ERROR:   lhs << "[error  ]"; break;
        case LOG_LEVEL_FATAL:   lhs << "[fatal  ]"; break;
    }

    return lhs;
}

struct XULOGMetadata
{
    std::string to_string() const
    {
        std::stringstream ss;
        ss << m_level << " " << m_location;
        return ss.str();
    }

    XULOGLevel     m_level;
    std::string    m_location;
};

class XULogger
{
public:

    virtual ~XULogger() {}

    virtual void write(const XULOGMetadata & meta, const std::string &text) = 0;

    void set_timestamp_enabled(bool enable = true) { m_timeStamp_enabled = enable; }

    bool timestamp_enabled() const { return m_timeStamp_enabled; }

protected:

    XULogger() : m_timeStamp_enabled(false)
    {

    }

    void write_default(std::ostream & stream, const XULOGMetadata & meta, const std::string & text)
    {
        stream << meta.m_level << " ";

        if (timestamp_enabled())
        {
            std::string timestr;
            XUGetTimeText(timestr);

            stream << timestr << " ";
        }

        stream << std::setw(40) << meta.m_location << ":   " << text << std::endl;
    }

private:

    bool m_timeStamp_enabled;

protected:

    XUMutex m_lock;
};

/* prints log messages in the application console window. */
class XULoggerConsole : public XULogger
{
public:

    void write(const XULOGMetadata & meta, const std::string & text)
    {
        XUMutexGuard mon(m_lock);

        std::stringstream ss;
        write_default(ss, meta, text);

#if defined(WIN32) || defined(_WIN32_WCE)
        std::cout << ss.str().c_str() << std::endl;
#endif
    }
};

class XULoggerFile : public XULogger
{
public:

    XULoggerFile()
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        file.open("XULog.log", std::ofstream::out | std::ofstream::trunc);
#elif defined(ANDROID)
        file.open("/sdcard/RPLog.log", std::ofstream::out | std::ofstream::trunc);
#endif
        if (!file.is_open())
        {
            /* file open failed */
        }
    }

    void write(const XULOGMetadata & meta, const std::string & text)
    {
        std::stringstream ss;
        write_default(ss, meta, text);

        {
            XUMutexGuard mon(m_lock);

            file << ss.str().c_str();
        }
    }

private:

    std::ofstream   file;
};

class XULoggerSystem : public XULogger
{
public:

    void write(const XULOGMetadata & meta, const std::string & text)
    {
        std::stringstream ss;
        write_default(ss, meta, text);
#if defined(ANDROID)
        android_LogPriority prio = ANDROID_LOG_DEFAULT;
        switch (meta.m_level)
        {
            case LOG_LEVEL_VERBOSE: prio = ANDROID_LOG_VERBOSE; break;
            case LOG_LEVEL_INFO:    prio = ANDROID_LOG_INFO;    break;
            case LOG_LEVEL_DEBUG:   prio = ANDROID_LOG_DEBUG;   break;
            case LOG_LEVEL_WARNING: prio = ANDROID_LOG_WARN;    break;
            case LOG_LEVEL_ERROR:   prio = ANDROID_LOG_ERROR;   break;
            case LOG_LEVEL_FATAL:   prio = ANDROID_LOG_FATAL;   break;
        }
#endif
        {
            XUMutexGuard mon(m_lock);
#if defined(WIN32) || defined(_WIN32_WCE)
            ::OutputDebugStringA(ss.str().c_str());
#elif defined(ANDROID)
            __android_log_print(prio, TAG, ss.str().c_str());
#endif
        }
    }
};

class XULogManager
{
public:
    static XULogManager * create_instance()
    {
        if (!m_instance)
        {
            XUMutexGuard mon(m_lock);

            if (!m_instance)
            {
                m_instance = new XULogManager();
            }
        }

        return m_instance;
    }

    static void destroy()
    {
        delete m_instance;
        m_instance = NULL;
    }

    template <typename T>
    T * create_logger ()
    {
        T * logger = new T();
        logger->set_timestamp_enabled(true);

        add_logger(logger);

        return logger;
    }

    void write(const XULOGMetadata & meta, const std::string & text)
    {
        std::vector<XULogger *>::iterator itr = m_loggers.begin();
        for (; itr != m_loggers.end(); ++itr)
        {
            (*itr)->write(meta, text);
        }
    }

private:

    void add_logger (XULogger * logger)
    {
        m_loggers.push_back(logger);
    }

protected:

    XULogManager()
    {

    }

    std::vector<XULogger *>  m_loggers;

private:

    static XUMutex        m_lock;
    static XULogManager * m_instance;
};

static
XULogManager * xu_logger_manager()
{
    return XULogManager::create_instance();
}

template<typename T>
void create_logger()
{
    xu_logger_manager()->create_logger<T>();
}

struct XULOGEntry
{
    XULOGEntry (XULOGLevel level, const std::string location)
    {
        m_metaData.m_level    = level;
        m_metaData.m_location = location;
    }

    ~XULOGEntry()
    {
        if (m_stream.tellp() > 0)
        {
            write_to_log();
        }
    }

    template <typename T>
    XULOGEntry & operator << (const T & rhs)
    {
        m_stream << rhs;
        return *this;
    }

    void write_to_log()
    {
        xu_logger_manager()->write (m_metaData, m_stream.str());
    }

    const XULOGMetadata & metadata() const { return m_metaData; }

private:

    XULOGMetadata          m_metaData;
    std::stringstream      m_stream;
};

/////////////////////////////////////////////////////////////////////////////
////

static
void XULOG_DEF(std::string location, XULOGLevel level, const char * fmt, ...)
{
    char log_msg[1024] = {0};

    va_list args;
    va_start(args, fmt);
#if defined(WIN32) || defined(_WIN32_WCE)
    vsprintf_s(log_msg, fmt, args);
#else
    vsprintf(log_msg, fmt, args);
#endif
    va_end(args);

    {
        XULOGEntry(level, location) << log_msg;
    }
}

/////////////////////////////////////////////////////////////////////////////
////

#ifndef XU_MIN_LOG_LEVEL
    #ifndef NDEBUG
        #define XU_MIN_LOG_LEVEL LOG_LEVEL_VERBOSE  ///< debug mode default is LOG_LEVEL_VERBOSE
    #else
        #define XU_MIN_LOG_LEVEL LOG_LEVEL_INFO     ///< release mode default is LOG_LEVEL_INFO
    #endif
#endif

#if (XU_MIN_LOG_LEVEL <= LOG_LEVEL_VERBOSE)
    #define XULOG_V(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_VERBOSE, __VA_ARGS__)
#else
    #define XULOG_V(...)   ((void)0)
#endif

#if (XU_MIN_LOG_LEVEL <= LOG_LEVEL_DEBUG)
    #define XULOG_D(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
    #define XULOG_D(...)   ((void)0)
#endif

#if (XU_MIN_LOG_LEVEL <= LOG_LEVEL_INFO)
    #define XULOG_I(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_INFO, __VA_ARGS__)
#else
    #define XULOG_I(...)   ((void)0)
#endif

#if( XU_MIN_LOG_LEVEL <= LOG_LEVEL_WARNING)
    #define XULOG_W(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_WARNING,__VA_ARGS__)
#else
    #define XULOG_W(...)   ((void)0)
#endif

#if (XU_MIN_LOG_LEVEL <= LOG_LEVEL_ERROR)
    #define XULOG_E(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_ERROR, __VA_ARGS__)
#else
    #define XULOG_E(...)   ((void)0)
#endif

#if (XU_MIN_LOG_LEVEL <= LOG_LEVEL_FATAL)
    #define XULOG_F(...)   XULOG_DEF (WHERE_AM_I, LOG_LEVEL_FATAL, __VA_ARGS__)
#else
    #define XULOG_F(...)   ((void)0)
#endif

#define XU_FOOTPRINT       XULOG_D("here i am!");

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XULOG_H___
