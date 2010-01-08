/*
 *  TestEventSystem.h
 *  Base
 *
 *  Created by Brent Wilson on 11/11/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#ifndef _TESTKEYBOARD_H_
#define _TESTKEYBOARD_H_
#include "Test.h"

class EventPump;
class TempListener;
class TestEventSystem : public Test<TestEventSystem> {
public:
    TestEventSystem(): Test<TestEventSystem>() {}
    static void RunTests();

private:
    static EventPump *_pump;
    static TempListener *_listener;
    static void TestKeyboardEvents();
};

#endif