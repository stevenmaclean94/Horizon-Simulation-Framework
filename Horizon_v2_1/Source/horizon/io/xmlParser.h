/*
<P> XML.c - implementation file for basic XML parser written in ANSI C++
for portability. It works by using recursion and a node tree for breaking
down the elements of an XML document.  </P>

@version     V2.09
@author      Frank Vanden Berghen

NOTE:

   If you add "#define STRICT_PARSING", on the first line of this file
   the parser will see the following XML-stream:
      <a><b>some text</b><b>other text    </a>
   as an error. Otherwise, this tring will be equivalent to:
      <a><b>some text</b><b>other text</b></a>

NOTE:

   If you add "#define APPROXIMATE_PARSING", on the first line of this file
   the parser will see the following XML-stream:
     <data name="n1">
     <data name="n2">
     <data name="n3" />
   as equivalent to the following XML-stream:
     <data name="n1" />
     <data name="n2" />
     <data name="n3" />
   This can be useful for badly-formed XML-streams but prevent the use
   of the following XML-stream (problem is: tags at contiguous levels
   have the same names):
     <data name="n1">
        <data name="n2">
            <data name="n3" />
        </data>
     </data>

BSD license:
Copyright (c) 2002, Frank Vanden Berghen
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

     Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     Neither the name of the Frank Vanden Berghen nor the
       names of its contributors may be used to endorse or promote products
	   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __INCLUDE_XML_NODE__
#define __INCLUDE_XML_NODE__

#include <stdlib.h>

#ifdef WIN32
#include <tchar.h>
#else
#include <wchar.h> // to have 'wcsrtombs' for ANSI version
                   // to have 'mbsrtowcs' for UNICODE version
#endif

#ifdef _UNICODE
// If you comment the next "define" line then the library will never "switch to" _UNICODE mode (16/32 bits per characters).
// This is useful when you get error messages like:
//    'XMLNode::openFileHelper' : cannot convert parameter 2 from 'const char [5]' to 'const wchar_t *'
// The _XMLUNICODE preprocessor variable force the XMLParser library into either utf16/32-mode (the variable must be
// defined) or utf8-mode(the variable must be undefined).
#define _XMLUNICODE
#endif

// Some common types for char set portable code
#ifdef _XMLUNICODE
    #ifndef WIN32
        #define _T(c) L ## c
    #endif
    #define XMLCSTR const wchar_t *
    #define XMLSTR  wchar_t *
    #define XMLCHAR wchar_t
#else
    #ifndef WIN32
        #define _T(c) c
    #endif
    #define XMLCSTR const char *
    #define XMLSTR  char *
    #define XMLCHAR char
#endif
#ifndef FALSE
    #define FALSE 0
#endif /* FALSE */
#ifndef TRUE
    #define TRUE 1
#endif /* TRUE */


namespace horizon {
	namespace io {

// Enumeration for XML parse errors.
typedef enum XMLError
{
    eXMLErrorNone = 0,
    eXMLErrorMissingEndTag,
    eXMLErrorEmpty,
    eXMLErrorFirstNotStartTag,
    eXMLErrorMissingTagName,
    eXMLErrorMissingEndTagName,
    eXMLErrorNoMatchingQuote,
    eXMLErrorUnmatchedEndTag,
    eXMLErrorUnmatchedEndClearTag,
    eXMLErrorUnexpectedToken,
    eXMLErrorInvalidTag,
    eXMLErrorNoElements,
    eXMLErrorFileNotFound,
    eXMLErrorFirstTagNotFound,
    eXMLErrorUnknownEscapeSequence,
    eXMLErrorCharConversionError,
    eXMLErrorCannotOpenWriteFile,
    eXMLErrorCannotWriteFile,

    eXMLErrorBase64DataSizeIsNotMultipleOf4,
    eXMLErrorBase64DecodeIllegalCharacter,
    eXMLErrorBase64DecodeTruncatedData,
    eXMLErrorBase64DecodeBufferTooSmall
} XMLError;

// Enumeration used to manage type of data. Use in conjunction with structure XMLNodeContents
typedef enum XMLElementType
{
    eNodeChild=0,
    eNodeAttribute=1,
    eNodeText=2,
    eNodeClear=3,
    eNodeNULL=4
} XMLElementType;

// Structure used to obtain error details if the parse fails.
typedef struct XMLResults
{
    enum XMLError error;
    int  nLine,nColumn;
} XMLResults;

// Structure for XML clear (unformatted) node (usually comments)
typedef struct {
    XMLCSTR lpszValue; XMLCSTR lpszOpenTag; XMLCSTR lpszCloseTag;
} XMLClear;

// Structure for XML attribute.
typedef struct {
    XMLCSTR lpszName; XMLCSTR lpszValue;
} XMLAttribute;

// The variable XMLClearTags below contains the clearTags recognized by the library
// You can modify the initialization of this variable inside the "xmlParser.cpp" file
// to change the clearTags that are currently recognized.
typedef struct {
    XMLCSTR lpszOpen; int openTagLen; XMLCSTR lpszClose;
} ALLXMLClearTag;
extern ALLXMLClearTag XMLClearTags[];

struct XMLNodeContents;

typedef struct XMLNode
{
  protected:

    struct XMLNodeDataTag;

    // protected constructor: use one of these four methods to get your first instance of XMLNode:
    //  - parseString
    //  - parseFile
    //  - openFileHelper
    //  - createXMLTopNode
    XMLNode(struct XMLNodeDataTag *pParent, XMLCSTR lpszName, int isDeclaration);

  public:

    // You can create your first instance of XMLNode with these 4 functions:
    // (see complete explanation of parameters below)

    static XMLNode createXMLTopNode(XMLCSTR lpszName, int isDeclaration=FALSE);
    static XMLNode parseString   (XMLCSTR      lpszXML, XMLCSTR tag=NULL, XMLResults *pResults=NULL);
    static XMLNode parseFile     (const char *filename, XMLCSTR tag=NULL, XMLResults *pResults=NULL);
    static XMLNode openFileHelper(const char *filename, XMLCSTR tag=NULL                           );

    // The tag parameter should be the name of the first tag inside the XML file.
    // If the tag parameter is omitted, the 3 functions return a node that represents
    // the head of the xml document including the declaration term (<? ... ?>).

    // If the XML document is corrupted:
    //   - The "openFileHelper" method will stop execution and display an error message on the console.
    //   - The 2 other methods will initialize the "pResults" variable with some information that
    //     can be used to trace the error.
    //   - If you still want to parse the file, you can use the APPROXIMATE_PARSING option as
    //     explained inside the note at the beginning of the "xmlParser.cpp" file.
    // You can have a user-friendly explanation of the parsing error with this function:
    static XMLCSTR getError(XMLError error);

    XMLCSTR getName();                                               // name of the node
    XMLCSTR getText(int i=0);                                        // return ith text field
    int nText();                                                     // nbr of text field
    XMLNode getChildNode(int i=0);                                   // return ith child node
    XMLNode getChildNode(XMLCSTR name, int i);                       // return ith child node with specific name
                                                                     //     (return an empty node if failing)
    XMLNode getChildNode(XMLCSTR name, int *i=NULL);                 // return next child node with specific name
                                                                     //     (return an empty node if failing)
    XMLNode getChildNodeWithAttribute(XMLCSTR tagName,               // return child node with specific name/attribute
                                      XMLCSTR attributeName,         //     (return an empty node if failing)
                                      XMLCSTR attributeValue=NULL,   //
                                      int *i=NULL);                  //
    int nChildNode(XMLCSTR name);                                    // return the number of child node with specific name
    int nChildNode();                                                // nbr of child node
    XMLAttribute getAttribute(int i=0);                              // return ith attribute
    XMLCSTR      getAttributeName(int i=0);                          // return ith attribute name
    XMLCSTR      getAttributeValue(int i=0);                         // return ith attribute name
    char  isAttributeSet(XMLCSTR name);                              // test if an attribute with a specific name is given
    XMLCSTR getAttribute(XMLCSTR name, int i);                       // return ith attribute content with specific name
                                                                     //     (return a NULL if failing)
    XMLCSTR getAttribute(XMLCSTR name, int *i=NULL);                 // return next attribute content with specific name
                                                                     //     (return a NULL if failing)
    int nAttribute();                                                // nbr of attribute
    XMLClear getClear(int i=0);                                      // return ith clear field (comment)
    int nClear();                                                    // nbr of clear field
    XMLSTR createXMLString(int nFormat=1, int *pnSize=NULL);         // create XML string starting from current XMLNode
                                                                     // if nFormat==0, no formatting is required
                                                                     // otherwise this returns an user friendly XML string from a
                                                                     // given element with appropriate white spaces and carriage returns.
                                                                     // if pnSize is given it returns the size in character of the string.
    XMLError writeToFile(const char *filename, const char *encoding=NULL, char nFormat=1);
                                                                     // save the content of an xmlNode inside a file.
                                                                     // the nFormat parameter has the same meaning as in the
                                                                     // createXMLString function. If "strictUTF8Parsing=1", the
                                                                     // the encoding parameter is ignored and always set to
                                                                     // "utf-8". If "_UNICODE=1", the encoding parameter is
                                                                     // ignored and always set to "utf-16".
    XMLNodeContents enumContents(int i);                             // enumerate all the different contents (child,text,
                                                                     //     clear,attribute) of the current XMLNode. The order
                                                                     //     is reflecting the order of the original file/string
    int nElement();                                                  // nbr of different contents for current node
    char isEmpty();                                                  // is this node Empty?
    char isDeclaration();                                            // is this node a declaration <? .... ?>

// to allow shallow/fast copy:
    ~XMLNode();
    XMLNode(const XMLNode &A);
    XMLNode& operator=( const XMLNode& A );

    XMLNode(): d(NULL){};
    static XMLNode emptyXMLNode;
    static XMLClear emptyXMLClear;
    static XMLAttribute emptyXMLAttribute;

    // The following functions allows you to create from scratch a XMLNode structure
    // Start by creating your top node with the "createXMLTopNode" function and then add new nodes with the "addChild" function.
    // static XMLNode createXMLTopNode();
    XMLNode       addChild(XMLCSTR lpszName, int isDeclaration=FALSE);
    XMLAttribute *addAttribute(XMLCSTR lpszName, XMLCSTR lpszValuev);
    XMLCSTR       addText(XMLCSTR lpszValue);
    XMLClear     *addClear(XMLCSTR lpszValue, XMLCSTR lpszOpen=XMLClearTags[0].lpszOpen, XMLCSTR lpszClose=XMLClearTags[0].lpszClose);
    XMLNode       addChild(XMLNode nodeToAdd);          // If the "nodeToAdd" has some parents, it will be detached
                                                        // from it's parents before being attached to the current XMLNode
    // Some update functions:
    XMLCSTR       updateName(XMLCSTR lpszName);                                                    // change node's name
    XMLAttribute *updateAttribute(XMLAttribute *newAttribute, XMLAttribute *oldAttribute);         // if the attribute to update is missing, a new one will be added
    XMLAttribute *updateAttribute(XMLCSTR lpszNewValue, XMLCSTR lpszNewName=NULL,int i=0);         // if the attribute to update is missing, a new one will be added
    XMLAttribute *updateAttribute(XMLCSTR lpszNewValue, XMLCSTR lpszNewName,XMLCSTR lpszOldName);  // set lpszNewName=NULL if you don't want to change the name of the attribute
                                                                                                   // if the attribute to update is missing, a new one will be added
    XMLCSTR       updateText(XMLCSTR lpszNewValue, int i=0);                                       // if the text to update is missing, a new one will be added
    XMLCSTR       updateText(XMLCSTR lpszNewValue, XMLCSTR lpszOldValue);                          // if the text to update is missing, a new one will be added
    XMLClear     *updateClear(XMLCSTR lpszNewContent, int i=0);                                    // if the clearTag to update is missing, a new one will be added
    XMLClear     *updateClear(XMLClear *newP,XMLClear *oldP);                                      // if the clearTag to update is missing, a new one will be added
    XMLClear     *updateClear(XMLCSTR lpszNewValue, XMLCSTR lpszOldValue);                         // if the clearTag to update is missing, a new one will be added

    // Some deletion functions:
    void deleteNodeContent();                           // delete the content of this XMLNode and the subtree
    void deleteAttribute(XMLCSTR lpszName);
    void deleteAttribute(int i=0);
    void deleteAttribute(XMLAttribute *anAttribute);
    void deleteText(int i=0);
    void deleteText(XMLCSTR lpszValue);
    void deleteClear(int i=0);
    void deleteClear(XMLClear *p);
    void deleteClear(XMLCSTR lpszValue);

    // The strings given as parameters for the following add and update methods (all these methods have
    // a name with the postfix "_WOSD" that means "WithOut String Duplication" ) will be free'd by the
    // XMLNode class. For example, it means that this is incorrect:
    //    xNode.addText_WOSD("foo");
    //    xNode.updateAttribute_WOSD("#newcolor" ,NULL,"color");
    // In opposition, this is correct:
    //    xNode.addText_WOSD(stringDup("foo"));
    //    xNode.updateAttribute_WOSD(stringDup("#newcolor"),NULL,"color");
    // Typically, you will never do:
    //    xNode.addText(base64Encode(...));
    // ... but rather:
    //    xNode.addText_WOSD(base64Encode(...));

    static XMLNode createXMLTopNode_WOSD(XMLCSTR lpszName, int isDeclaration=FALSE);
    XMLNode        addChild_WOSD(XMLCSTR lpszName, int isDeclaration=FALSE);
    XMLAttribute  *addAttribute_WOSD(XMLCSTR lpszName, XMLCSTR lpszValue);
    XMLCSTR        addText_WOSD(XMLCSTR lpszValue);
    XMLClear      *addClear_WOSD(XMLCSTR lpszValue, XMLCSTR lpszOpen=XMLClearTags[0].lpszOpen, XMLCSTR lpszClose=XMLClearTags[0].lpszClose);

    XMLCSTR        updateName_WOSD(XMLCSTR lpszName);
    XMLAttribute  *updateAttribute_WOSD(XMLAttribute *newAttribute, XMLAttribute *oldAttribute);
    XMLAttribute  *updateAttribute_WOSD(XMLCSTR lpszNewValue, XMLCSTR lpszNewName=NULL,int i=0);
    XMLAttribute  *updateAttribute_WOSD(XMLCSTR lpszNewValue, XMLCSTR lpszNewName,XMLCSTR lpszOldName);
    XMLCSTR        updateText_WOSD(XMLCSTR lpszNewValue, int i=0);
    XMLCSTR        updateText_WOSD(XMLCSTR lpszNewValue, XMLCSTR lpszOldValue);
    XMLClear      *updateClear_WOSD(XMLCSTR lpszNewContent, int i=0);
    XMLClear      *updateClear_WOSD(XMLClear *newP,XMLClear *oldP);
    XMLClear      *updateClear_WOSD(XMLCSTR lpszNewValue, XMLCSTR lpszOldValue);

    static void setGlobalOptions(char guessUnicodeChars=1, char strictUTF8Parsing=1);
    //
    // First of all, you most-probably will never have to change these 2 global parameters.
    // About the "guessUnicodeChars" parameter:
    //     If "guessUnicodeChars=1" and if this library is compiled in UNICODE mode, then the
    //     "parseFile" and "openFileHelper" functions will test if the file contains ASCII
    //     characters. If this is the case, then the file will be loaded and converted in memory to
    //     UNICODE before being parsed. If "guessUnicodeChars=0", no conversion will
    //     be performed.
    //
    //     If "guessUnicodeChars=1" and if this library is compiled in ASCII/UTF8 mode, then the
    //     "parseFile" and "openFileHelper" functions will test if the file contains UNICODE
    //     characters. If this is the case, then the file will be loaded and converted in memory to
    //     ASCII/UTF8 before being parsed. If "guessUnicodeChars=0", no conversion will
    //     be performed
    //
    //     Sometime, it's useful to set "guessUnicodeChars=0" to disable any conversion
    //     because the test to detect the file-type (ASCII/UTF8 or UNICODE) may fail (rarely).
    //
    // About the "strictUTF8Parsing" parameter:
    //     If "strictUTF8Parsing=0" then we assume that all characters have the same length of 1 byte.
    //     If "strictUTF8Parsing=1" then the characters have different lengths (from 1 byte to 4 bytes)
    //     depending on the content of the first byte of the character.

    static char guessUTF8ParsingParameterValue(void *buffer, int bufLen, char useXMLEncodingAttribute=1);
    // First of all, you most-probably will never have to use this function.
    // This function try to guess if the character encoding is UTF-8. It then returns the appropriate
    // value of the global parameter "strictUTF8Parsing" described above. The guess is based on the
    // content of a buffer of length "bufLen" bytes that contains the first bytes (minimum 25
    // bytes; 200 bytes is a good value) of the file to be parsed. The "openFileHelper" function is
    // using this function to automatically compute the value of the "strictUTF8Parsing" global parameter.
    // There are several heuristics used to do the guess. One of the heuristic is based on the "encoding"
    // attribute. The original XML specifications forbids to use this attribute to do the guess but
    // you can still use it if you set "useXMLEncodingAttribute" to 1.

  protected:

// these are functions and structures used internally by the XMLNode class (don't bother about them):

      typedef struct XMLNodeDataTag // to allow shallow copy and "intelligent/smart" pointers (automatic delete):
      {
          XMLCSTR                lpszName;        // Element name (=NULL if root)
          int                    nChild,          // Num of child nodes
                                 nText,           // Num of text fields
                                 nClear,          // Num of Clear fields (comments)
                                 nAttribute,      // Num of attributes
                                 isDeclaration;   // Whether node is an XML declaration - '<?xml ?>'
          struct XMLNodeDataTag  *pParent;        // Pointer to parent element (=NULL if root)
          XMLNode                *pChild;         // Array of child nodes
          XMLCSTR                *pText;          // Array of text fields
          XMLClear               *pClear;         // Array of clear fields
          XMLAttribute           *pAttribute;     // Array of attributes
          int                    *pOrder;         // order in which the child_nodes,text_fields,clear_fields and
          int                    ref_count;       // for garbage collection (smart pointers)
      } XMLNodeData;
      XMLNodeData *d;

  private:

      static void destroyCurrentBuffer(XMLNodeData *d);
      int ParseClearTag(void *pXML, void *pClear);
      int ParseXMLElement(void *pXML);
      void addToOrder(int index, int type);
      static int CreateXMLStringR(XMLNodeData *pEntry, XMLSTR lpszMarker, int nFormat);
      static void *enumContent(XMLNodeData *pEntry,int i, XMLElementType *nodeType);
      static int nElement(XMLNodeData *pEntry);
      static void removeOrderElement(XMLNodeData *d, XMLElementType t, int index);
      static void exactMemory(XMLNodeData *d);
      static void detachFromParent(XMLNodeData *d);
} XMLNode;

// This structure is given by the function "enumContents".
typedef struct XMLNodeContents
{
    // This dictates what's the content of the XMLNodeContent
    enum XMLElementType type;
    // should be an union to access the appropriate data.
    // compiler does not allow union of object with constructor... too bad.
    XMLNode child;
    XMLAttribute attrib;
    XMLCSTR text;
    XMLClear clear;

} XMLNodeContents;

// Duplicate (copy in a new allocated buffer) the source string. This is
// a very handy function when used with all the "XMLNode::*_WOSD" functions.
// (If (cbData!=0) then cbData is the number of chars to duplicate)
XMLSTR stringDup(XMLCSTR source, int cbData=0);

// The 3 following functions are processing strings so that all the characters
// &,",',<,> are replaced by their XML equivalent: &amp;, &quot;, &apos;, &lt;, &gt;.
// These 3 functions are useful when creating from scratch an XML file using the
// "printf", "fprintf", "cout",... functions. If you are creating from scratch an
// XML file using the provided XMLNode class you cannot use these functions (the
// XMLNode class does the processing job for you during rendering). The second
// function ("toXMLStringFast") allows you to re-use the same output buffer
// for all the conversions so that only a few memory allocations are performed.
// If the output buffer is too small to contain the resulting string, it will
// be enlarged.
XMLSTR toXMLString(XMLCSTR source);
XMLSTR toXMLStringFast(XMLSTR *destBuffer,int *destSz, XMLCSTR source);

// you should not use this one (there is a possibility of "destination-buffer-overflow"):
XMLSTR toXMLString(XMLSTR dest,XMLCSTR source);

// Below are four functions that allows you to include any binary data (images, sounds,...)
// into an XML document using "Base64 encoding". These 4 functions are completely
// separated from the rest of the xmlParser library and can be removed without any problem.
// To include some binary data into an XML file, you must convert the binary data into
// standard text (using "base64Encode"). To retrieve the original binary data from the
// encoded text included inside the XML file use "base64Decode". Alternatively, these
// functions can also be used to "encrypt/decrypt" some critical data contained inside
// the XML.

    // Returns a string containing the base64 encoding of "inByteLen" bytes from "inByteBuf"
    // If "formatted" parameter is true, then there will be a carriage-return every 72 chars.
    // The string length (in char) is optionally returned inside "outStringLen".
    XMLSTR base64Encode(char *inByteBuf, unsigned int inByteLen, char formatted=0, unsigned int *outStringLen=NULL);

    // returns a pointer to a newly allocated region containing the binary data decoded from "inString"
    // If "inString" is malformed NULL will be returned
    char* base64Decode(XMLCSTR inString, unsigned int *outByteLen=NULL, XMLError *xe=NULL);

    // returns the number of bytes which will be decoded from "inString".
    unsigned int base64DecodeSize(XMLCSTR inString, XMLError *xe=NULL);

    // decodes data into "outByteBuf". You need to provide the size of in "inMaxByteBuflen"
    // If "outByteBuf" is not large enough or if data is malformed, then "FALSE"
    // will be returned; otherwise "TRUE"
    char base64Decode(XMLCSTR inString, char *outByteBuf, unsigned int inMaxByteBuflen, XMLError *xe=NULL);


}} //end namespace horizon:io

#endif