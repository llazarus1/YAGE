/*
 *  TestCPPLogger.h
 *  Base
 *
 *  Created by Brent Wilson on 10/22/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#ifndef _TESTCPPLOGGER_H_
#define _TESTCPPLOGGER_H_
#include "Test.h"

class TestCPPLogger : public Test<TestCPPLogger> {
public:
    TestCPPLogger(): Test<TestCPPLogger>() {}
    static void RunTests();

private:
    static void TestPretext();
    static void TestLogTarget();
    static void TestLogLevel();
    static void TestClearFile();
    static void TestIndentLevel();
    static void TestTags();

};

#endif
