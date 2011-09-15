/*
   This file is part of VHMsg written by Edward Fast at 
   University of Southern California's Institute for Creative Technologies.
   http://www.ict.usc.edu
   Copyright 2008 Edward Fast, University of Southern California

   VHMsg is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   VHMsg is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with VHMsg.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma warning ( push )
#pragma warning ( disable: 4251 4275 )  // warnings in vector because of log4cxx classes
#include "vhcl.h"
#pragma warning (pop)

#include <string>

#include <stdio.h>
#include <stdarg.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#if defined (TARGET_OS_IPHONE)  || defined (TARGET_IPHONE_SIMULATOR)
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#pragma warning ( push )
#pragma warning ( disable: 4231 )
#ifndef SBM_IPHONE
#include "log4cxx/logger.h"
#include "log4cxx/appenderskeleton.h"
#include "log4cxx/consoleappender.h"
#include "log4cxx/fileappender.h"
#include "log4cxx/nt/outputdebugstringappender.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/transcoder.h"
#endif
#pragma warning (pop)


#define VHCL_LOG_FUNCTION_ENABLED  1


using namespace vhcl::Log;


// global instance of a logger
Logger vhcl::Log::g_log;


#ifndef SBM_IPHONE
void LOG( const char * message, ... )
{
#if VHCL_LOG_FUNCTION_ENABLED
   va_list argList;
   va_start( argList, message );
   vhcl::Log::g_log.vLog( message, argList );
   va_end( argList );
#endif
}


// this is a log4cxx appender that will call OnMessage() on custom Listeners that don't have native log4cxx appenders already written
namespace vhcl {
namespace Log {
class VHCLLog4CxxAppender : public log4cxx::AppenderSkeleton
{
   private:
      Logger * logger;

   public:
      explicit VHCLLog4CxxAppender( Logger * logger ) { this->logger = logger; }

      bool requiresLayout() const { return true; }

      virtual void close() {}

      virtual void append( const log4cxx::spi::LoggingEventPtr & event, log4cxx::helpers::Pool & p )
      {
         log4cxx::LogString buf;
         layout->format( buf, event, p );
         LOG4CXX_ENCODE_CHAR( str, buf );

         for ( size_t i = 0; i < logger->m_listeners.size(); i++ )
         {
            logger->m_listeners[ i ]->OnMessage( str );
         }
      }
};
} }


FileListener::FileListener( const char * filename )
{
   m_filename = filename;

   log4cxx::LayoutPtr layout( new log4cxx::PatternLayout( LOG4CXX_STR( "%m%n" ) ) );
   LOG4CXX_DECODE_CHAR( m_filenameLog, m_filename );
   log4cxx::FileAppender * appender = new log4cxx::FileAppender( layout, m_filenameLog, false );
   appender->setName( LOG4CXX_STR( "vhclFileAppender" ) );
   log4cxx::Logger::getRootLogger()->addAppender( appender );
};


FileListener::~FileListener()
{
}


void FileListener::OnMessage( const std::string & message )
{
   // we're now letting log4cxx handle the message
}


DebuggerListener::DebuggerListener()
{
#if defined(WIN_BUILD)
   log4cxx::LayoutPtr layout( new log4cxx::PatternLayout( LOG4CXX_STR( "%m%n" ) ) );
   log4cxx::nt::OutputDebugStringAppender * appender = new log4cxx::nt::OutputDebugStringAppender();
   appender->setName( LOG4CXX_STR( "vhclOutputDebugStringAppender" ) );
   appender->setLayout( layout );
   log4cxx::Logger::getRootLogger()->addAppender( appender );
#endif
}


DebuggerListener::~DebuggerListener()
{
}


void DebuggerListener::OnMessage( const std::string & message )
{
   // we're now letting log4cxx handle the message
}


StdoutListener::StdoutListener()
{
   log4cxx::LayoutPtr layout( new log4cxx::PatternLayout( LOG4CXX_STR( "%m%n" ) ) );
   log4cxx::ConsoleAppender * appender = new log4cxx::ConsoleAppender( layout );
   appender->setName( LOG4CXX_STR( "vhclConsoleAppender" ) );
   log4cxx::Logger::getRootLogger()->addAppender( appender );
}


StdoutListener::~StdoutListener()
{
}


void StdoutListener::OnMessage( const std::string & message )
{
   // we're now letting log4cxx handle the message
}




Logger::Logger()
{
   m_configuredByFile = false;
   AddVHCLAppender();
}


Logger::~Logger()
{
   RemoveAllListeners();
   log4cxx::Logger::getRootLogger()->removeAllAppenders();  // do this again to remove VHCL appender
}


void Logger::AddListener( Listener * listener )
{
   if ( m_configuredByFile )
   {
      RemoveAllListeners();
      m_configuredByFile = false;
   }

   m_listeners.push_back( listener );
}


void Logger::RemoveListener( Listener * listener )
{
   std::vector<Listener *>::iterator iter = m_listeners.end();
   for (iter = m_listeners.begin(); iter != m_listeners.end(); iter++)
   {
      if ((*iter) == listener)
         break;
   }

   if (iter != m_listeners.end())
      m_listeners.erase(iter);
}


void Logger::RemoveAllListeners()
{
   for ( size_t i = 0; i < m_listeners.size(); i++ )
   {
      // don't call delete until we more formally describe the ownership policy
      //delete m_listeners[ i ];
   }
   m_listeners.clear();

   log4cxx::Logger::getRootLogger()->removeAllAppenders();
   AddVHCLAppender();
}


void Logger::UseFileConfig( const char * filename )
{
   RemoveAllListeners();
   log4cxx::Logger::getRootLogger()->removeAllAppenders();  // do this again to remove VHCL appender

   log4cxx::PropertyConfigurator::configure( filename );
   m_configuredByFile = true;
}


bool Logger::IsEnabled() const
{
   // returns false if there are no listeners

   if ( m_configuredByFile )
   {
      if ( log4cxx::Logger::getRootLogger()->getAllAppenders().empty() )
         return false;
   }
   else
   {
      if ( m_listeners.empty() )
         return false;
   }

   return true;
}


void Logger::Log( const char * message, ... )
{
   if ( !IsEnabled() )
      return;

   va_list argList;
   va_start( argList, message );
   std::string result = vhcl::vFormat( message, argList );
   va_end( argList );

   LOG4CXX_DEBUG( log4cxx::Logger::getRootLogger(), result.c_str() );
}


void Logger::vLog( const char * message, va_list argPtr )
{
   if ( !IsEnabled() )
      return;

   std::string result = vhcl::vFormat( message, argPtr );

   LOG4CXX_DEBUG( log4cxx::Logger::getRootLogger(), result.c_str() );
}


void Logger::AddVHCLAppender()
{
   VHCLLog4CxxAppender * appender = new VHCLLog4CxxAppender( this );
   appender->setName( LOG4CXX_STR( "vhcl" ) );
   appender->setLayout( new log4cxx::PatternLayout( LOG4CXX_STR( "%m%n" ) ) );
   log4cxx::Logger::getRootLogger()->addAppender( appender );
}

#else
void LOG( const char * message, ... )
{
    va_list argList;
    va_start( argList, message );
    vhcl::Log::g_log.vLog( message, argList );
    va_end( argList );
}

FileListener::FileListener( const char * filename )
{
    
};


FileListener::~FileListener()
{
}


void FileListener::OnMessage( const std::string & message )
{
    // we're now letting log4cxx handle the message
}


DebuggerListener::DebuggerListener()
{
    
}


DebuggerListener::~DebuggerListener()
{
}


void DebuggerListener::OnMessage( const std::string & message )
{
    // we're now letting log4cxx handle the message
}


StdoutListener::StdoutListener()
{
    
}


StdoutListener::~StdoutListener()
{
}


void StdoutListener::OnMessage( const std::string & message )
{
    // we're now letting log4cxx handle the message
}




Logger::Logger()
{
}


Logger::~Logger()
{
    
}


void Logger::AddListener( Listener * listener )
{
    
}


void Logger::RemoveListener( Listener * listener )
{
    
}


void Logger::RemoveAllListeners()
{
    
}


void Logger::UseFileConfig( const char * filename )
{
}


bool Logger::IsEnabled() const
{
    
    return true;
}


void Logger::Log( const char * message, ... )
{
}


void Logger::vLog( const char * message, va_list argPtr )
{
    if ( !IsEnabled() )
        return;
    
    std::string result = vhcl::vFormat( message, argPtr );
    
    printf("%s", result.c_str());
}


void Logger::AddVHCLAppender()
{
}
#endif
