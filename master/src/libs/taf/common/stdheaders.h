#pragma once

//C headers
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

//C++ headers
#ifdef WINDOWS
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <bitset>

#include <vector>
#include <map>
#include <unordered_map>

#include <chrono>
#include <regex>
#include <any>
#include <thread>
#include <stdarg.h>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

#include <Windows.h>

#else // gcc
#include <bits/stdc++.h>
#endif