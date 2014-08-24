/*
 *   Copyright 2014 Emeric Verschuur <emericv@openihs.org>
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *		   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

%{
#define YYDEBUG 1

#include "ltype.h"
#include "nodevariant.h"
#include "nodelist.h"
#include "nodemap.h"
#include "nodemodule.h"
#include "nodeintf.h"
#include "noderoot.h"
#include "nodeparams.h"
#include <parser.hh>
#include "scanner.h"
#include "driver.h"
#include <qt4/QtCore/QVariant>
#define yylex driver.m_scanner.yylex

%}

%token              TEND 0          "end of file"

%token <node>       TINCLUDE        "#include<...>"

%token              TMODULE         "module"
%token              TINTERFACE      "interface"

%token              TVOID           "void"
%token              TBOOLEAN        "boolean"
%token              TCHAR           "char"
%token              TWCHAR          "wchar"
%token              TFLOAT          "float"
%token              TDOUBLE         "double"
%token              TLONG           "long"
%token              TSHORT          "short"
%token              TOCTET          "octet"
%token              TSTRING         "string"
%token              TOBJECT         "Object"
%token              TDATETIME       "DateTime"
%token              TANY            "any"

%token              TUNSIGNED       "unsigned"
%token              TCONST          "const"
%token              TREADONLY       "readonly"
%token              TATTRIBUTE      "attribute"

%token              TENUM           "enum"
%token              TSTRUCT         "struct"
%token              TUNION          "union"
%token              TSWITCH         "switch"
%token              TCASE           "case"
%token              TDEFAULT        "default"
%token              TONEWAY         "oneway"
%token              TIN             "in"
%token              TOUT            "out"
%token              TINOUT          "inout"
%token              TSEQUENCE       "sequence"
%token              TEXCEPTION      "exception"
%token              TRAISES         "raises"

%token              TTYPEDEF        "typedef"

%token <node>       TSTRINGVAL      "string value"
%token <node>       TSYMBOL         "symbol"
%token <node>       TNUMBER         "number value"
%token <node>       TINTVAL         "integer value"
%token <node>       TFLOATVAL       "float value"
%token <node>       TVARIANT        "variant"

%token              TSYNERRESC      "invalid escaped character"
%token              TSYNERRUNI      "invalid unicode character"
%token              TSYNERR         "invalid character"

%start DOCUMENT

%type <node>        DOCUMENT DOCUMENT_ELTS DOCUMENT_ELT MODULE_ELTS MODULE_ELT MODULE_HEADER MODULE ENUM STRUCT
%type <node>        EXCEPTION STRUCT_ELTS TYPEDEF SYMBOL_LIST SYMBOL INTERFACE INTERFACE_ELTS INTERFACE_ELT SEQUENCE RET_TYPE
%type <node>        FIELD ATTRIBUTE CONSTANT EXPRESSION METHOD METHOD_HEADER METHOD_FOOTER PARAMETERS PARAMETER PARAMETER_DIR
%type <node>        VALUE ATTRIBUTE_QUAL INTERFACE_PARENT INTERFACE_HEADER TYPE ARRAYOF UNION CASES CASE

%left   '+' '-'
%left   '*' '/' '%'

%language "C++"
%define namespace "idlparser"
%define parser_class_name "Parser"
/*
%define api.namespace {idlparser}
%define parser_class_name {Parser}
*/
%parse-param {idlparser::Driver &driver}

%error-verbose
%debug

%%

DOCUMENT : DOCUMENT_ELTS TEND                     {YYACCEPT;}
    ;

DOCUMENT_ELTS : DOCUMENT_ELTS DOCUMENT_ELT        {/* NOTHING TO DO */}
    | DOCUMENT_ELT                                {/* NOTHING TO DO */}
    ;

DOCUMENT_ELT : MODULE_ELT                         {/* NOTHING TO DO */}
    | TINCLUDE                                    {driver.include($1->str());}
    ;

MODULE : MODULE_HEADER '{' MODULE_ELTS '}' ';'    {driver.pop();}
    ;

MODULE_HEADER : TMODULE TSYMBOL                   {driver.push(new NodeModule(driver, $2));}
    ;

MODULE_ELTS : MODULE_ELTS MODULE_ELT              {/* NOTHING TO DO */}
    | MODULE_ELT                                  {/* NOTHING TO DO */}
    ;

MODULE_ELT : INTERFACE                            {$$ = $1;}
    | MODULE                                      {$$ = $1;}
    | ENUM                                        {$$ = $1;}
    | UNION                                       {$$ = $1;}
    | STRUCT                                      {$$ = $1;}
    | TYPEDEF                                     {$$ = $1;}
    | EXCEPTION                                   {$$ = $1;}
    ;

ENUM : TENUM TSYMBOL '{' SYMBOL_LIST '}' ';'      {if (!driver.appendError("Error: enum definition not supported")) YYABORT;}
    ;

STRUCT : TSTRUCT TSYMBOL '{' STRUCT_ELTS '}' ';'  {if (!driver.appendError("Error: struct definition not supported")) YYABORT;}
    ;

STRUCT_ELTS : STRUCT_ELTS FIELD ';'               {/* NOTHING TO DO */}
    | FIELD ';'                                   {/* NOTHING TO DO */}
    ;

TYPEDEF : TTYPEDEF TYPE TSYMBOL ';'               {if (!driver.appendError("Error: typedef definition not supported")) YYABORT;}
    | TTYPEDEF TYPE TSYMBOL '[' TNUMBER ']' ';'   {if (!driver.appendError("Error: typedef definition not supported")) YYABORT;}
    ;

UNION : TUNION TSYMBOL TSWITCH '(' TYPE ')' '{' CASES '}' ';'
                                                  {if (!driver.appendError("Error: union not supported")) YYABORT;}
    ;

CASES : CASES CASE                                {/* NOTHING TO DO */}
    | CASE                                        {/* NOTHING TO DO */}
    ;

CASE : TCASE TSYMBOL ':' STRUCT_ELTS              {/* NOTHING TO DO */}
    ;

EXCEPTION : TEXCEPTION TSYMBOL '{' STRUCT_ELTS '}' ';'
                                                  {if (!driver.appendError("Error: exception definition not supported")) YYABORT;}
    ;

SYMBOL_LIST : SYMBOL_LIST ',' SYMBOL              {$$ = $1; $$->append($3->val());}
    | SYMBOL                                      {$$ = (new NodeList())->append($1->val());}
    ;

SYMBOL : SYMBOL ':' ':' TSYMBOL                   {$$ = new NodeVariant($1->str() + "::" + $4->str());}
    | TSYMBOL                                     {$$ = $1;}
    ;

INTERFACE : INTERFACE_HEADER '{' INTERFACE_ELTS '}' ';'
                                                  {driver.pop();}
    ;

INTERFACE_HEADER : TINTERFACE SYMBOL INTERFACE_PARENT
                                                  {driver.push(new NodeIntf(driver, $2, $3));}
    ;

INTERFACE_PARENT : ':' SYMBOL_LIST                {$$ = $2;}
    |                                             {$$ = new NodeList();}
    ;

INTERFACE_ELTS : INTERFACE_ELTS INTERFACE_ELT     {/* NOTHING TO DO */}
    | INTERFACE_ELT                               {/* NOTHING TO DO */}
    ;

INTERFACE_ELT : ATTRIBUTE                         {driver.m_curCtx->append($1);}
    | METHOD                                      {driver.m_curCtx->append($1);}
    | CONSTANT                                    {driver.m_curCtx->append($1);}
    ;

SEQUENCE : TSEQUENCE '<' TYPE '>'                 {if (!driver.appendError("Error: sequence not supported")) YYABORT;}
    | TSEQUENCE '<' TYPE ',' TNUMBER '>'          {if (!driver.appendError("Error: sequence not supported")) YYABORT;}
    ;

ARRAYOF : TYPE '[' ']'                            {if (!driver.appendError("Error: array other than octet[] not supported")) YYABORT;}
    | TYPE '[' TNUMBER ']'                        {if (!driver.appendError("Error: array with fixed size not supported")) YYABORT;}
    ;

RET_TYPE : TYPE                                   {$$ = $1;}
    | TVOID                                       {$$ = new NodeVariant(VTYPE_VOID);}
    ;

TYPE : TOBJECT                                    {$$ = new NodeVariant(VTYPE_ANY);}
    | TANY                                        {$$ = new NodeVariant(VTYPE_ANY);}
    | TBOOLEAN                                    {$$ = new NodeVariant(VTYPE_BOOLEAN);}
    | TOCTET                                      {$$ = new NodeVariant(VTYPE_BYTE);}
    | TCHAR                                       {$$ = new NodeVariant(VTYPE_BYTE);}
    | TWCHAR                                      {$$ = new NodeVariant(VTYPE_BYTE);}
    | TUNSIGNED TSHORT                            {$$ = new NodeVariant(VTYPE_UINT32);}
    | TSHORT                                      {$$ = new NodeVariant(VTYPE_INT32);}
    | TUNSIGNED TLONG                             {$$ = new NodeVariant(VTYPE_UINT32);}
    | TLONG                                       {$$ = new NodeVariant(VTYPE_INT32);}
    | TUNSIGNED TLONG TLONG                       {$$ = new NodeVariant(VTYPE_UINT64);}
    | TLONG TLONG                                 {$$ = new NodeVariant(VTYPE_INT64);}
    | TFLOAT                                      {$$ = new NodeVariant(VTYPE_DOUBLE);}
    | TDOUBLE                                     {$$ = new NodeVariant(VTYPE_DOUBLE);}
    | TSTRING                                     {$$ = new NodeVariant(VTYPE_STRING);}
    | TOCTET '[' ']'                              {$$ = new NodeVariant(VTYPE_BYTEARRAY);}
    | TDATETIME                                   {$$ = new NodeVariant(VTYPE_DATETIME);}
    | SEQUENCE                                    {if (!driver.appendError("Error: sequence not supported")) YYABORT;}
    | TSYMBOL                                     {if (!driver.appendError("Error: Complex/user defined type not supported")) YYABORT;}
    | ARRAYOF                                     {$$ = new NodeVariant("");}
    ;

FIELD : TYPE SYMBOL '<' TNUMBER '>'               {if (!driver.appendError("Error: array definition not supported")) YYABORT;}
    | TYPE SYMBOL '[' TNUMBER ']'                 {if (!driver.appendError("Error: array definition not supported")) YYABORT;}
    | TYPE SYMBOL '[' ']'                         {if (!driver.appendError("Error: array definition not supported")) YYABORT;}
    | TYPE SYMBOL                                 {$$ = (new NodeMap())->insert(KNODE_DTYPE, $1->val())->insert(KNODE_SNAME, $2->val());}
    ;

ATTRIBUTE : ATTRIBUTE_QUAL TATTRIBUTE FIELD ';'   {$$ = $3->insert(KNODE_WRITABLE, $1->val())->insert(KNODE_TYPE, NTYPE_ATTR);}
    ;

ATTRIBUTE_QUAL : TREADONLY                        {$$ = new NodeVariant(false);}
    |                                             {$$ = new NodeVariant(true);}
    ;

CONSTANT : TCONST FIELD '=' EXPRESSION ';'        {$$ = $2; $$->insert(KNODE_TYPE, NTYPE_CONST)->insert(KNODE_VALUE, $4->val());}
    ;

EXPRESSION : '(' EXPRESSION ')'                   {$$ = $2;}
    | EXPRESSION '+' EXPRESSION                   {if (!opexec($$, '+', $1, $3, driver)) YYABORT;}
    | EXPRESSION '-' EXPRESSION                   {if (!opexec($$, '-', $1, $3, driver)) YYABORT;}
    | EXPRESSION '*' EXPRESSION                   {if (!opexec($$, '*', $1, $3, driver)) YYABORT;}
    | EXPRESSION '/' EXPRESSION                   {if (!opexec($$, '/', $1, $3, driver)) YYABORT;}
    | EXPRESSION '%' EXPRESSION                   {if (!opexec($$, '%', $1, $3, driver)) YYABORT;}
    | VALUE                                       {$$ = $1;}
    ;

VALUE : TVARIANT                                  {$$ = $1;}
    | TINTVAL                                     {$$ = $1;}
    | TNUMBER                                     {$$ = $1;}
    | TSYMBOL                                     {$$ = driver.m_curCtx->resolve($1, NTYPE_CONST);}
    ;

METHOD : METHOD_HEADER RET_TYPE SYMBOL '(' PARAMETERS ')' METHOD_FOOTER ';'
                                                  {$$ = (new NodeMap())->insert(KNODE_TYPE, NTYPE_METHOD)->insert(KNODE_DTYPE, $2->val())->insert(KNODE_SNAME, $3->val())->insert(KNODE_PARAMS, $5->list());}
    ;

METHOD_HEADER : TONEWAY                           {if (!driver.appendError("Error: unsupported oneway keyword")) YYABORT;}
    |                                             {/* NOTHING TO DO */}
    ;

METHOD_FOOTER : TRAISES '(' SYMBOL_LIST ')'       {if (!driver.appendError("Error: exception raise not supported")) YYABORT;}
    |                                             {/* NOTHING TO DO */}
    ;

PARAMETERS : PARAMETERS ',' PARAMETER             {if (!$1->append($3)) YYABORT; $$ = $1;}
    | PARAMETER                                   {$$ = new NodeParams(driver); if (!$$->append($1)) YYABORT;}
    | TVOID                                       {$$ = new NodeParams(driver);}
    |                                             {$$ = new NodeParams(driver);}
    ;

PARAMETER : PARAMETER_DIR FIELD                   {$$ = $2->insert(KNODE_DIRECTION, $1->val());}
    ; 

PARAMETER_DIR : TIN                               {$$ = new NodeVariant(PDIR_IN);}
    | TOUT                                        {$$ = new NodeVariant(PDIR_OUT);}
    | TINOUT                                      {$$ = new NodeVariant(PDIR_INOUT);}
    |                                             {$$ = new NodeVariant(PDIR_IN);}
    ;


%%
