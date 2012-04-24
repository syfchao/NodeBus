/*
    Copyright (c) 2012, Emeric Verschuur <emericv@openihs.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the OpenIHS.org nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Emeric Verschuur <emericv@openihs.org> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Emeric Verschuur <emericv@openihs.org> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

%{

#include "globals.h"
#include <parser.hh>
#include "scanner.h"
#include "driver.h"
#include "routines.h"
#define yylex driver.scanner.yylex

using namespace std;

%}

%union {
  string_t *str;
  variant_t *node;
  array_t *array;
  object_t *object;
}

%token              TEND 0          "end of file"

%token              TOBJBEGIN       "object begin '{'"
%token              TOBJEND         "object end '}'"
%token              TARRBEGIN       "array begin '['"
%token              TARREND         "array end ']'"
%token              TFIELDSEP       "object field separator ':'"
%token              TELEMENTSEP     "object/array element separator ','"
%token <str>        TSTRING         "string"
%token <node>       TVARIANT        "variant"

%token              TSYNERRESC      "invalid escaped character"
%token              TSYNERRUNI      "invalid unicode character"

%start ROOT

%type <object>      MAP
%type <array>       LIST
%type <node>        ROOT VARIANT

%language "C++"
%define namespace "jsonparser"
%define parser_class_name "Parser"
%parse-param {Driver &driver}

%error-verbose

%%

ROOT : VARIANT                          {driver.result = $$ = $1;}
    ;

VARIANT : TOBJBEGIN MAP TOBJEND         {$$ = r::map2variant($2);}
    | TARRBEGIN LIST TARREND            {$$ = r::list2variant($2);}
    | TOBJBEGIN TOBJEND                 {$$ = r::map2variant(new object_t());}
    | TARRBEGIN TARREND                 {$$ = r::list2variant(new array_t());}
    | TSTRING                           {$$ = r::string2variant($1);}
    | TVARIANT                          {$$ = $1;}
    ;

MAP : MAP TELEMENTSEP
        TSTRING TFIELDSEP VARIANT       {$$ = r::mapAppendVariant($1, $3, $5);}
    | TSTRING TFIELDSEP VARIANT         {$$ = r::mapAppendVariant(NULL, $1, $3);}
    ;

LIST : LIST TELEMENTSEP VARIANT         {$$ = r::listAppendVariant($1, $3);}
    | VARIANT                           {$$ = r::listAppendVariant(NULL, $1);}
    ;

%%
