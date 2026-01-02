/*
 * Copyright (c) 2022 Jan Privara
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
