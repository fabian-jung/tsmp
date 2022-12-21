# Changelog

## 1.0.6

test


## 1.0.5

- Refactor tooling backend


## 1.0.4

- fix json_decode throwing exceptions

## 1.0.3


- enable reflection for targets with multiple source files

## 1.0.2


- Add support for compiling with clang
- Integrate clang into the ci pipeline
- Add additional testing for the internal classes

## 1.0.1

- Bugfix ci pipeline for the push to main hook
- Bugfix Catch2 could not be loaded when tsmp is loaded via FetchContents

## 1.0.0

The very first Release of the TSMP library.

- Reflect types via tsmp::reflect
- Generic accsess on members of values with tsmp::introspect
- Reflect enums via tsmp::enum_value_adapter
- JSON encode and decode with tsmp::to_json and tsmp::from_json
