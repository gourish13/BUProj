# Business Aligned Project

## Overview

There are two process running in two seperate user namely, `dbreader` and `datareceiver`.

**dbreader** - Will be running using OS_USER_1 which will have DB access. DBreader will try to
connect to the database and extract sample records and convert it to JSON before sending it to
Datareceiver. After every successful transaction DBreader cleans the buffer and the old data is
discarded.

**satareceiver** - This will be another agent looking for the data read by dbreader. It will be
running using OS_USER_2 on the same Linux host. It creates a JSON data file where this
process is appending 100 records per transfer after checksum validation.

--------------------------------------------

## Dependencies

1. C compiler `gcc`, `clang` or similar
2. MySQL C library 
3. Json-C library

## Setup

## Build and Install
