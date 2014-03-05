## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir)/include -I@srcdir@/include

bin_SCRIPTS=csv_parser-config

lib_LTLIBRARIES = libcsv_parser.la

libcsv_parser_la_SOURCES = csv_parser.cpp include/csv_parser/csv_parser.hpp

libcsv_parser_la_includedir=$(includedir)/csv_parser

libcsv_parser_la_include_HEADERS = include/csv_parser/csv_parser.hpp

