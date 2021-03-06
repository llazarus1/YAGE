/*
 *  Log.h
 *  Base
 *
 *  Created by Brent Wilson on 8/14/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#ifndef _LOGSTREAM_H_
#define _LOGSTREAM_H_
#include "Base.h"

/*! This is a class used to aid in the output of information. It is meant as an actual
 *  replacement for the built in std::cout and std::cerr functions. This class
 *  automatically places spaces between passed data, automatically adds an endline
 *  after you're done outputing. Further features include support for a small set of
 *  string replacement tags for easy inserting of time, class names, line numbers, etc...,
 *  built in indentation, leveled output (trace, debug, info, warn, error), and
 *  redirection of output to a file or the console. The debug level may be set at compile
 *  time allowing certain messages to be completely removed from the code resulting in
 *  absolutely zero performance loss (the downside to this is the necessity of macros).
 *
 *  This class is not meant to be used directly, rather the utility macros below make use
 *  of this class to generate output. The 5 macros currently supported are:
 *   - Trace
 *   - Debug
 *   - Info
 *   - Warn
 *   - Error
 *
 *  Each macro has its own trace level associated with it. As stated before, output
 *  levels can be adjusted at compile and run time (though compile acts as a hard cull).
 *  By setting the log level to InfoMessage, only messages of level Info, Warn, and Error
 *  will be output. Usage for these methods are as follows:
 *    Debug("Some debug output.");
 *
 *  This logger will allow certain built in text replacements to make certain output
 *  easier. Tags to be automatically replaced are marked with the % sign followed by a
 *  letter to indicate what the tag should be replaced with. Here is a list of currently
 *  supported tags:
 *    %f - File
 *    %t - Time
 *    %l - Line
 *
 *  Just like C++'s cout and cerr methods, this logger makes use of ostreams. As such,
 *  variable insertion is very similar to cout and cerr. Here is an example of adding
 *  variables to output and using the automatic replacement tags:
 *
 *  int a = 5, b = 1;
 *  Debug("[%t] " << a << " * " << b << " = " << a); // Outputs: [18:30] 5 * 1 = 5
 *
 *  If you always want the time prepended to your output, you may also call SetPretext
 *  passing in "[%t]".
 *
 * \brief A helper class meant to aid logging.
 * \author Brent Wilson
 * \date 11/7/06
 * \todo Add a %c for class
 * \todo Add separators? (ie ---------Init---------)
 * \todo Write a test to ensure the correct << operator is being called.
 * \todo Enforce a line width?
 * \todo Should indents be before, or after the pretext? Perhaps a variable?!
 * \todo Add logging to a stream (would help testing greatly).
 * \todo Make pretext and posttexts a list that you can push and pop on/off of. */

// LOGCHAN: Logging channels for Content target.
enum { ContentChannel = 0x04 };
#define ContentInfo InfoC(ContentChannel, stream)

// LOGCHAN: Logging channels for Render target.
enum { RenderChannel = 0x08};
#define RenderInfo(stream) InfoC(RenderChannel, stream)
enum { GraphicsMemoryChannel = 0x10 };
#define GraphicsMemInfo(stream) InfoC(GraphicsMemoryChannel, stream)

// LOGCHAN: Logging channels for Engine target.
enum { SceneManagementChannel = 0x20 };
#define SceneMInfo(stream) InfoC(SceneManagementChannel, stream)
enum { StateManagementChannel = 0x40 };
#define StateMInfo(stream) InfoC(StateManagementChannel, stream)
enum { DisplayChannel = 0x80 };
#define DisplayInfo(stream) InfoC(DisplayChannel, stream)
enum { AudioChannel = 0x100 };
#define AudioInfo(stream) InfoC(AudioChannel, stream)

// LOGCHAN: Logging channels for Mountainhome target.
enum { TerrainChannel = 0x200 };
#define TerrainInfo(stream) InfoC(TerrainChannel, stream)
enum { UIChannel = 0x400 };
#define UIInfo(stream) InfoC(UIChannel, stream)
enum { RubyBindingsChannel = 0x800 };
#define RubyBindingInfo(stream) InfoC(RubyBindingsChannel, stream)
enum { WorldgenChannel = 0x1000 };
#define WorldgenInfo(stream) InfoC(WorldgenChannel, stream)
enum { GameLogicChannel = 0x2000 };
#define GameLogicInfo(stream) InfoC(GameLogicChannel, stream)

typedef int LogChannel;

class LogStream {
public :
    /*! LogType enumerates the different log levels available to the system. */
    enum LogType {
        TraceMessage,   /*!< Used for tracing the codepath.                          */
        DebugMessage,   /*!< Used to help find errors in the code.                   */
        InfoMessage,    /*!< Used for generic information that would be helpful.     */
        WarningMessage, /*!< Used when questionable behavior arises.                 */
        ErrorMessage,   /*!< Used to log errors that could have major repercussions. */
    };

    /*! LOGCHAN: Logging channels for Base target. */
    /*! Channels for other targets will be scattered around, each defined in their target.
        Hopefully the keyword LOGCHAN will help find these channel definitions, which
        each must be a unique power of two to avoid collisions.
    */
    enum {
        DefaultChannel      = 0x01,       /*! If no channel is specified. */
        MathChannel         = 0x02,       /*! Math logging. */
    };

    /*! LogDestination enumerates where log output may be directed. */
    enum LogDestination {
        None = 0,            /*!< Logs nowhere at all.                  */
        Console = 1,         /*!< Logs only to the console.             */
        Disk = 2,            /*!< Logs only to disk.                    */
        All = Console | Disk /*!< Logs both to the console and to disk. */
    };

public:
    /*! Sets where log output should be directed. */
    static void SetLogDestination(LogDestination dest);

    /*! Sets the pretext string that is printed as the object is being created.
     * \param text The text to print out during object construction (Usually spaces). */
    static void SetPretext(const std::string &text);

    /*! Sets an indent level for output. By setting the indent level, leading whitespace
     *  is added to all output to aid in clarity.
     * \param level The number of indentations to place before output.
     * \sa setIndentSize(int size) */
    static void SetIndentLevel(int level);

    /*! Increases the indent level by a certain ammount.
     * \param number The number to increment the current level by.
     * \sa setIndentSize(int size) */
    static void IncrementIndent(int number = 1);

    /*! Decreases the indent level by a certain ammount.
     * \param number The number to decrement the current level by.
     * \sa setIndentSize(int size) */
    static void DecrementIndent(int number = 1);

    /*! Sets the number of spaces per indentation.
     * \param size The number of spaces in a single indentation.
     * \return The previous value of the indent. */
    static int SetIndentSize(int size);

    /*! Sets the a parameter that determines if LogType 'Error' messages will cause the
     *  program to abort once the message has been printed out. This may make debugging
     *  certain errors easier.
     * \param abort The new value to determine if errors result in an abort.
     * \return The previous value of the flag. */
    static bool SetBreakOnError(bool abort);

    /*! This function is used to determine where program output is directed. Output can be
     *  sent to a log file or standard out.
     * \param filename Sets the file to log to. If this value is NULL, logging will go to
     *        standard out. */
    static void SetLogTarget(const std::string &filename);

    /*! This allows the user to control just how much output is generated by the program.
     *  'DebugMessage' will generate the most output, while 'FatalMessage' will generate
     *  the least and 'NoOutput' will not generate any at all.
     * \param level The minimum level of output to actually log. */
    static void SetLogLevel(LogType level);

    /*! Add and remove channels to display. Only the DefaultChannel will print by default. */
    static void EnableLogChannel(LogChannel channel);
    static void DisableLogChannel(LogChannel channel);

    static void EnableAllChannels();
    static void DisableAllChannels();

    /*! Returns whether a given channel or combination of channels is enabled. */
    static bool IsChannelEnabled(LogChannel channel);

    /*! Sets the internal variable that determines if the filenames used to substitute the
     *  %f tag should be trimmed down or not.
     * \param val The new value. */
    static void SetTrimFilenames(bool val);

    /*! This will delete the old log file. */
    static void ClearLogFile();

    /*! Returns the appropriate log stream based on the given level and the internal
     *  level and sets internal state needed to print out properly.
     * \param type    If the given log level is equal or less than the internal one, the
     *                active log stream is returned. If not, a null log stream is returned
     *                which outputs nowhere at all
     * \param newline Sets whether not not the stream should print the data on a new line
     *                or the same one it was using previously.
     * \param file    The name of the file the output is coming from.
     * \param line    The line number the output is coming from.
     * \return        The appropriate LogStream object with the correct state. */
    static LogStream& GetLogStream(LogType type, bool newline, const std::string &file, int line);

    /*! Causes all relevant streams to be flushed. */
    static void Flush();

    /*! Inserts a simple separator to the log file to help organize things. */
    static void InsertSeparator();

protected:
    static void PrintColorTagsForLevel(LogType level);

    static void DeleteOutStream();

    static void CreateOutStream();

    /*! Scans through a string and replaces all of the tags with their correct values.
     * \param original The string to scan through.
     * \param temp If any tags are found, the new string will be stored in temp.
     * \param file The name of the file the output is coming from.
     * \param line The line number the output is coming from.
     * \return     If there are any tags, a reference to temp is returned, otherwise a
     *             reference to original is returned */
    static const std::string& ReplaceTags(const std::string &original, std::string &temp, const std::string &file, int line);

    /*! This function is used to test the result of a simple output operation. It simply
     *  outputs the given string at the given log level and returns the result.
     * \param string The string to output AND the result of the operation.
     * \param output The level at which to output the string. */
    static void PrintAndCheckResult(char* string, LogType output = InfoMessage);

    /*! This function simply reads the current log file into the string. Meant for testing
     *  purposes.
     * \param string The result of the file read. */
    static void CheckResult(char* string);

    friend class TestCPPLogger;

    static std::string Logfile; /*!< The name of the file to log to (stdout if "").     */
    static std::string Pretext; /*!< Text to prepend to log output.                     */
    static bool BreakOnError;   /*!< Determines if the program dies after an error.     */
    static LogType LogLevel;    /*!< The last level to allow logging at.                */
    static LogChannel ActiveLogChannels;    /*!< The channels currently active.         */
    static LogDestination Dest; /*!< Where log output should be directed. Default: All  */
    static int IndentLevel;     /*!< The number of indentations to prepend output with. */
    static int IndentSize;      /*!< The size of the indentations prepended to output.  */
    static bool TrimFilenames;  /*!< Determines if the log should shorten filenames.    */
    static LogStream *OutStream;/*!< Outputs based on the static state at its creation. */
    static LogStream *NilStream;/*!< Outputs to nowhere at all.                         */

public:
    /*! Initializes LogStream class based on the static state. */
    LogStream(std::ostream *console, const std::string &filename);

    /*! Prints the posttext, an endline if specified in the constructor and kills the
     *  program on fatal messages (unless AbortOnError has been set to false). */
    ~LogStream();

    /*! This is the actual stream operator that allows output. Any data passed to this
     *  function will be sent on to the current log target. After every piece of data it
     *  prints, a ' ' is inserted for ease of use. Note that strings and char*s are
     *  handled in overloaded methods. The only difference is they do a tag replacement
     *  before printing out.
     * \param rhs The data type to be logged. Can be any type that implements the '<<'
     *        operator for output.
     * \return A reference to the LogStream for function chaining. */
    template <typename T> LogStream& operator<<(const T &rhs);

    /*! Flushes all active, internal streams. */
    void flush();

protected:
    std::ostream *_console; /*!< The stream to which output is redirected. */
    std::fstream *_file;    /*!< The stream to which output is redirected. */

};

template <typename T> LogStream& LogStream::operator<<(const T &rhs) {
    // \todo Figure out why I have to cast this to a subclass to get it to compile.
    if (Dest & Disk    && _file   ) { (*(std::ostream*)_file) << rhs; }
    if (Dest & Console && _console) { (*_console) << rhs; }
    return *this;
}

#define LogAtLevel(to_log, level, channel, newline) \
    LogAtLevelWithFL(to_log, newline, level, channel, __FILE__, __LINE__);

#define LogAtLevelWithFL(to_log, newline, level, channel, file, line) \
    do { \
        if (LogStream::IsChannelEnabled(channel)) { \
            std::ostringstream stream; \
            stream << to_log; \
            std::string str_val = stream.str(); \
             \
            std::list<std::string> log_list; \
            tokenize <std::list<std::string> > (str_val, "\n", log_list); \
             \
            std::list<std::string>::iterator itr; \
            for(itr = log_list.begin(); itr != log_list.end(); itr++) { \
                LogStream::GetLogStream(level, newline, file, line) << (*itr); LogStream::Flush(); \
            } \
        } \
    } while(false)

#if !defined(TRACE_LEVEL) || TRACE_LEVEL < 1
#   define Trace(stream) TraceC(LogStream::DefaultChannel, stream)
#   define TraceC(channel, stream) LogAtLevel(stream, LogStream::TraceMessage, channel, true)
#else
#   define Trace(stream) do {} while(false)
#   define TraceC(channel, stream) do {} while(false)
#endif

#if !defined(TRACE_LEVEL) || TRACE_LEVEL < 2
#   define Debug(stream) DebugC(LogStream::DefaultChannel, stream)
#   define DebugC(channel, stream) LogAtLevel(stream, LogStream::DebugMessage, channel, true)
#else
#   define Debug(stream) do {} while(false)
#   define DebugC(channel, stream) do {} while(false)
#endif

#if !defined(TRACE_LEVEL) || TRACE_LEVEL < 3
#   define Info(stream) InfoC(LogStream::DefaultChannel, stream)
#   define InfoC(channel, stream) LogAtLevel(stream, LogStream::InfoMessage, channel, true)
#else
#   define Info(stream) do {} while(false)
#   define InfoC(channel, stream) do {} while(false)
#endif

#if !defined(TRACE_LEVEL) || TRACE_LEVEL < 4
#   define Warn(stream) WarnC(LogStream::DefaultChannel, stream)
#   define WarnC(channel, stream) LogAtLevel(stream, LogStream::WarningMessage, channel, true)
#else
#   define Warn(stream) do {} while(false)
#   define WarnC(channel, stream) do {} while(false)
#endif

#if !defined(TRACE_LEVEL) || TRACE_LEVEL < 5
#   define Error(stream) ErrorC(LogStream::DefaultChannel, stream)
#   define ErrorC(channel, stream) LogAtLevel(stream, LogStream::ErrorMessage, channel, true)
#else
#   define Error(stream) do {} while(false)
#   define ErrorC(channel, stream) do {} while(false)
#endif

#endif
