/*
    Copyright (c) 2017, Ole Stauning
    All rights reserved.

    Use of this source code is governed by a GPLv3-style license that can be found
    in the LICENSE file.
*/

#include "text_tool.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

#include <locale>
#include <codecvt>

namespace ByteFountain
{
	namespace text_tool
	{
		namespace
		{
			struct cp_unicode_convert
			{
				const wchar_t* m_cp;
				cp_unicode_convert(const wchar_t* cp):m_cp(cp){}
				wchar_t operator()(const char c) const { return m_cp[(int)c]; }
			};
		}
		
		bool try_cp_to_unicode(const wchar_t* cp, const std::string& cptext, std::wstring& res)
		{
			res.resize(cptext.size());
			cp_unicode_convert conv(cp);
			std::transform(cptext.begin(),cptext.end(),res.begin(),conv);
			for(size_t i=0;i<res.size();++i) if (res[i]==0) return false;
			return true;
		}

		std::wstring cp_to_unicode(const wchar_t* cp, const std::string& cptext)
		{
			if (0==cp) return utf8_to_unicode(cptext); // decode as utf8 if cp=0...
			std::wstring res;
			res.resize(cptext.size());
			cp_unicode_convert conv(cp);
			std::transform(cptext.begin(),cptext.end(),res.begin(),conv);
			return res;
		}
		std::string unicode_to_utf8(const std::wstring& wstr)
		{
			size_t len=0;
			
			wchar_t w;
			for(size_t i=0;i<wstr.size();++i)
			{
				w=wstr[i];
				if (w<0x0080) len++;
				else if (w<0x0800) len+=2;
				else len+=3;
			}

			std::string utf8str;
			utf8str.resize(len);

			size_t j = 0;
			for(size_t i=0;i<wstr.size();++i)
			{
				w=wstr[i];
				if ( w < 0x0080 )
					utf8str[j++] = ( unsigned char ) w;
				else if ( w < 0x0800 )
				{
					utf8str[j++] = 0xc0 | (( w ) >> 6 );
					utf8str[j++] = 0x80 | (( w ) & 0x3f );
				}
				else
				{
					utf8str[j++] = 0xe0 | (( w ) >> 12 );
					utf8str[j++] = 0x80 | (( ( w ) >> 6 ) & 0x3f );
					utf8str[j++] = 0x80 | (( w ) & 0x3f );
				}
			}

			return utf8str;
		}

	
		bool try_utf8_to_unicode(const std::string& utf8str, std::wstring& res)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> ucs4conv("Error!",L"Error!");
			try
			{
				res = ucs4conv.from_bytes(utf8str);
			} 
			catch(const std::range_error& /*e*/) 
			{
//				std::cerr<<"FAILED CONVERTING UTF8 TO UNICODE: "<<e.what()<<std::endl;
				return false;
			}
			return true;
		}
		std::wstring utf8_to_unicode(const std::string& utf8str)
		{
			std::wstring res;
			if (!try_utf8_to_unicode(utf8str,res))
			{
				std::cerr<<"FAILED CONVERTING UTF8 TO UNICODE"<<std::endl;
				exit(-1);
			}
			return res;
		}

		std::wstring utf8_to_unicode(const char* utf8str) 
		{ 
			if (utf8str==0) return std::wstring();
			return utf8_to_unicode(std::string(utf8str));
		}

		std::string cp_to_utf8(const wchar_t* cp, const std::string& cpstr)
		{
			size_t len=0;
			
			wchar_t w;
			for(size_t i=0;i<cpstr.size();++i)
			{
				w=cp[(unsigned char)cpstr[i]];
				if (w==0) continue;
				else if (w<0x0080) len++;
				else if (w<0x0800) len+=2;
				else len+=3;
			}

			std::string utf8str;
			utf8str.resize(len);

			size_t j = 0;
			for(size_t i=0;i<cpstr.size();++i)
			{
				w=cp[(unsigned char)cpstr[i]];
				if (w==0) continue;
				else if ( w < 0x0080 )
					utf8str[j++] = ( unsigned char ) w;
				else if ( w < 0x0800 )
				{
					utf8str[j++] = 0xc0 | (( w ) >> 6 );
					utf8str[j++] = 0x80 | (( w ) & 0x3f );
				}
				else
				{
					utf8str[j++] = 0xe0 | (( w ) >> 12 );
					utf8str[j++] = 0x80 | (( ( w ) >> 6 ) & 0x3f );
					utf8str[j++] = 0x80 | (( w ) & 0x3f );
				}
			}
			
			return utf8str;
		}
		std::string strip_utf8_to_ascii(const std::string& utf8str, const unsigned char pad_char)
		{
			std::string res;
			res.resize(utf8str.size());
			size_t j=0;
			for(size_t i=0;i<utf8str.size();++i)
			{
				if ((unsigned char)utf8str[i]<0x80) res[j++]=utf8str[i];
				else 
				{
					res[j++]=pad_char;
					if ((unsigned char)utf8str[i]<0xe0) i+=1;
					else if ((unsigned char)utf8str[i]<0xf0) i+=2;
					else if ((unsigned char)utf8str[i]<0xf8) i+=3;
					else if ((unsigned char)utf8str[i]<0xfc) i+=4;
					else i+=5;
				}
			}
			res.resize(j);
			return res;
		}
		std::string strip_cp_to_ascii(const std::string& cpstr, const unsigned char pad_char)
		{
			std::string res;
			res.resize(cpstr.size());
			for(size_t i=0;i<cpstr.size();++i)
			{
				if ((unsigned char)cpstr[i]<0x80) res[i]=cpstr[i];
				else res[i]=pad_char;
			}
			return res;
		}

		bool is_ascii_text(const std::string& str)
		{
			for(size_t i=0;i<str.size();++i)
			{
				if (str[i]<32 || str[i]>126) return false;
			}
			return true;
		}
		
		//
		//    Name:     cp437_DOSLatinUS to Unicode table
		//    Unicode version: 2.0
		//    Table version: 2.00
		//    Table format:  Format A
		//    Date:          04/24/96
		//    Contact: Shawn.Steele@microsoft.com
		//                  
		//    General notes: none
		//
		//    Format: Three tab-separated columns
		//        Column //1 is the cp437_DOSLatinUS code (in hex)
		//        Column //2 is the Unicode (in hex as 0xXXXX)
		//        Column //3 is the Unicode name (follows a comment sign, '//')
		//
		//    The entries are in cp437_DOSLatinUS order
		wchar_t cp437[]=
		{
		/*0x00*/0x0000,//NULL
		/*0x01*/0x0000,// N/A 0x0001,//START OF HEADING
		/*0x02*/0x0000,// N/A 0x0002,//START OF TEXT
		/*0x03*/0x0000,// N/A 0x0003,//END OF TEXT
		/*0x04*/0x0000,// N/A 0x0004,//END OF TRANSMISSION
		/*0x05*/0x0000,// N/A 0x0005,//ENQUIRY
		/*0x06*/0x0000,// N/A 0x0006,//ACKNOWLEDGE
		/*0x07*/0x0000,// N/A 0x0007,//BELL
		/*0x08*/0x0000,// N/A 0x0008,//BACKSPACE
		/*0x09*/0x0000,// N/A 0x0009,//HORIZONTAL TABULATION
		/*0x0a*/0x0000,// N/A 0x000a,//LINE FEED
		/*0x0b*/0x0000,// N/A 0x000b,//VERTICAL TABULATION
		/*0x0c*/0x0000,// N/A 0x000c,//FORM FEED
		/*0x0d*/0x0000,// N/A 0x000d,//CARRIAGE RETURN
		/*0x0e*/0x0000,// N/A 0x000e,//SHIFT OUT
		/*0x0f*/0x0000,// N/A 0x000f,//SHIFT IN
		/*0x10*/0x0000,// N/A 0x0010,//DATA LINK ESCAPE
		/*0x11*/0x0000,// N/A 0x0011,//DEVICE CONTROL ONE
		/*0x12*/0x0000,// N/A 0x0012,//DEVICE CONTROL TWO
		/*0x13*/0x0000,// N/A 0x0013,//DEVICE CONTROL THREE
		/*0x14*/0x0000,// N/A 0x0014,//DEVICE CONTROL FOUR
		/*0x15*/0x0000,// N/A 0x0015,//NEGATIVE ACKNOWLEDGE
		/*0x16*/0x0000,// N/A 0x0016,//SYNCHRONOUS IDLE
		/*0x17*/0x0000,// N/A 0x0017,//END OF TRANSMISSION BLOCK
		/*0x18*/0x0000,// N/A 0x0018,//CANCEL
		/*0x19*/0x0000,// N/A 0x0019,//END OF MEDIUM
		/*0x1a*/0x0000,// N/A 0x001a,//SUBSTITUTE
		/*0x1b*/0x0000,// N/A 0x001b,//ESCAPE
		/*0x1c*/0x0000,// N/A 0x001c,//FILE SEPARATOR
		/*0x1d*/0x0000,// N/A 0x001d,//GROUP SEPARATOR
		/*0x1e*/0x0000,// N/A 0x001e,//RECORD SEPARATOR
		/*0x1f*/0x0000,// N/A 0x001f,//UNIT SEPARATOR
		/*0x20*/0x0020,//SPACE
		/*0x21*/0x0021,//EXCLAMATION MARK
		/*0x22*/0x0022,//QUOTATION MARK
		/*0x23*/0x0023,//NUMBER SIGN
		/*0x24*/0x0024,//DOLLAR SIGN
		/*0x25*/0x0025,//PERCENT SIGN
		/*0x26*/0x0026,//AMPERSAND
		/*0x27*/0x0027,//APOSTROPHE
		/*0x28*/0x0028,//LEFT PARENTHESIS
		/*0x29*/0x0029,//RIGHT PARENTHESIS
		/*0x2a*/0x002a,//ASTERISK
		/*0x2b*/0x002b,//PLUS SIGN
		/*0x2c*/0x002c,//COMMA
		/*0x2d*/0x002d,//HYPHEN-MINUS
		/*0x2e*/0x002e,//FULL STOP
		/*0x2f*/0x002f,//SOLIDUS
		/*0x30*/0x0030,//DIGIT ZERO
		/*0x31*/0x0031,//DIGIT ONE
		/*0x32*/0x0032,//DIGIT TWO
		/*0x33*/0x0033,//DIGIT THREE
		/*0x34*/0x0034,//DIGIT FOUR
		/*0x35*/0x0035,//DIGIT FIVE
		/*0x36*/0x0036,//DIGIT SIX
		/*0x37*/0x0037,//DIGIT SEVEN
		/*0x38*/0x0038,//DIGIT EIGHT
		/*0x39*/0x0039,//DIGIT NINE
		/*0x3a*/0x003a,//COLON
		/*0x3b*/0x003b,//SEMICOLON
		/*0x3c*/0x003c,//LESS-THAN SIGN
		/*0x3d*/0x003d,//EQUALS SIGN
		/*0x3e*/0x003e,//GREATER-THAN SIGN
		/*0x3f*/0x003f,//QUESTION MARK
		/*0x40*/0x0040,//COMMERCIAL AT
		/*0x41*/0x0041,//LATIN CAPITAL LETTER A
		/*0x42*/0x0042,//LATIN CAPITAL LETTER B
		/*0x43*/0x0043,//LATIN CAPITAL LETTER C
		/*0x44*/0x0044,//LATIN CAPITAL LETTER D
		/*0x45*/0x0045,//LATIN CAPITAL LETTER E
		/*0x46*/0x0046,//LATIN CAPITAL LETTER F
		/*0x47*/0x0047,//LATIN CAPITAL LETTER G
		/*0x48*/0x0048,//LATIN CAPITAL LETTER H
		/*0x49*/0x0049,//LATIN CAPITAL LETTER I
		/*0x4a*/0x004a,//LATIN CAPITAL LETTER J
		/*0x4b*/0x004b,//LATIN CAPITAL LETTER K
		/*0x4c*/0x004c,//LATIN CAPITAL LETTER L
		/*0x4d*/0x004d,//LATIN CAPITAL LETTER M
		/*0x4e*/0x004e,//LATIN CAPITAL LETTER N
		/*0x4f*/0x004f,//LATIN CAPITAL LETTER O
		/*0x50*/0x0050,//LATIN CAPITAL LETTER P
		/*0x51*/0x0051,//LATIN CAPITAL LETTER Q
		/*0x52*/0x0052,//LATIN CAPITAL LETTER R
		/*0x53*/0x0053,//LATIN CAPITAL LETTER S
		/*0x54*/0x0054,//LATIN CAPITAL LETTER T
		/*0x55*/0x0055,//LATIN CAPITAL LETTER U
		/*0x56*/0x0056,//LATIN CAPITAL LETTER V
		/*0x57*/0x0057,//LATIN CAPITAL LETTER W
		/*0x58*/0x0058,//LATIN CAPITAL LETTER X
		/*0x59*/0x0059,//LATIN CAPITAL LETTER Y
		/*0x5a*/0x005a,//LATIN CAPITAL LETTER Z
		/*0x5b*/0x005b,//LEFT SQUARE BRACKET
		/*0x5c*/0x005c,//REVERSE SOLIDUS
		/*0x5d*/0x005d,//RIGHT SQUARE BRACKET
		/*0x5e*/0x005e,//CIRCUMFLEX ACCENT
		/*0x5f*/0x005f,//LOW LINE
		/*0x60*/0x0060,//GRAVE ACCENT
		/*0x61*/0x0061,//LATIN SMALL LETTER A
		/*0x62*/0x0062,//LATIN SMALL LETTER B
		/*0x63*/0x0063,//LATIN SMALL LETTER C
		/*0x64*/0x0064,//LATIN SMALL LETTER D
		/*0x65*/0x0065,//LATIN SMALL LETTER E
		/*0x66*/0x0066,//LATIN SMALL LETTER F
		/*0x67*/0x0067,//LATIN SMALL LETTER G
		/*0x68*/0x0068,//LATIN SMALL LETTER H
		/*0x69*/0x0069,//LATIN SMALL LETTER I
		/*0x6a*/0x006a,//LATIN SMALL LETTER J
		/*0x6b*/0x006b,//LATIN SMALL LETTER K
		/*0x6c*/0x006c,//LATIN SMALL LETTER L
		/*0x6d*/0x006d,//LATIN SMALL LETTER M
		/*0x6e*/0x006e,//LATIN SMALL LETTER N
		/*0x6f*/0x006f,//LATIN SMALL LETTER O
		/*0x70*/0x0070,//LATIN SMALL LETTER P
		/*0x71*/0x0071,//LATIN SMALL LETTER Q
		/*0x72*/0x0072,//LATIN SMALL LETTER R
		/*0x73*/0x0073,//LATIN SMALL LETTER S
		/*0x74*/0x0074,//LATIN SMALL LETTER T
		/*0x75*/0x0075,//LATIN SMALL LETTER U
		/*0x76*/0x0076,//LATIN SMALL LETTER V
		/*0x77*/0x0077,//LATIN SMALL LETTER W
		/*0x78*/0x0078,//LATIN SMALL LETTER X
		/*0x79*/0x0079,//LATIN SMALL LETTER Y
		/*0x7a*/0x007a,//LATIN SMALL LETTER Z
		/*0x7b*/0x007b,//LEFT CURLY BRACKET
		/*0x7c*/0x007c,//VERTICAL LINE
		/*0x7d*/0x007d,//RIGHT CURLY BRACKET
		/*0x7e*/0x007e,//TILDE
		/*0x7f*/0x007f,//DELETE
		/*0x80*/0x00c7,//LATIN CAPITAL LETTER C WITH CEDILLA
		/*0x81*/0x00fc,//LATIN SMALL LETTER U WITH DIAERESIS
		/*0x82*/0x00e9,//LATIN SMALL LETTER E WITH ACUTE
		/*0x83*/0x00e2,//LATIN SMALL LETTER A WITH CIRCUMFLEX
		/*0x84*/0x00e4,//LATIN SMALL LETTER A WITH DIAERESIS
		/*0x85*/0x00e0,//LATIN SMALL LETTER A WITH GRAVE
		/*0x86*/0x00e5,//LATIN SMALL LETTER A WITH RING ABOVE
		/*0x87*/0x00e7,//LATIN SMALL LETTER C WITH CEDILLA
		/*0x88*/0x00ea,//LATIN SMALL LETTER E WITH CIRCUMFLEX
		/*0x89*/0x00eb,//LATIN SMALL LETTER E WITH DIAERESIS
		/*0x8a*/0x00e8,//LATIN SMALL LETTER E WITH GRAVE
		/*0x8b*/0x00ef,//LATIN SMALL LETTER I WITH DIAERESIS
		/*0x8c*/0x00ee,//LATIN SMALL LETTER I WITH CIRCUMFLEX
		/*0x8d*/0x00ec,//LATIN SMALL LETTER I WITH GRAVE
		/*0x8e*/0x00c4,//LATIN CAPITAL LETTER A WITH DIAERESIS
		/*0x8f*/0x00c5,//LATIN CAPITAL LETTER A WITH RING ABOVE
		/*0x90*/0x00c9,//LATIN CAPITAL LETTER E WITH ACUTE
		/*0x91*/0x00e6,//LATIN SMALL LIGATURE AE
		/*0x92*/0x00c6,//LATIN CAPITAL LIGATURE AE
		/*0x93*/0x00f4,//LATIN SMALL LETTER O WITH CIRCUMFLEX
		/*0x94*/0x00f6,//LATIN SMALL LETTER O WITH DIAERESIS
		/*0x95*/0x00f2,//LATIN SMALL LETTER O WITH GRAVE
		/*0x96*/0x00fb,//LATIN SMALL LETTER U WITH CIRCUMFLEX
		/*0x97*/0x00f9,//LATIN SMALL LETTER U WITH GRAVE
		/*0x98*/0x00ff,//LATIN SMALL LETTER Y WITH DIAERESIS
		/*0x99*/0x00d6,//LATIN CAPITAL LETTER O WITH DIAERESIS
		/*0x9a*/0x00dc,//LATIN CAPITAL LETTER U WITH DIAERESIS
		/*0x9b*/0x00a2,//CENT SIGN
		/*0x9c*/0x00a3,//POUND SIGN
		/*0x9d*/0x00a5,//YEN SIGN
		/*0x9e*/0x20a7,//PESETA SIGN
		/*0x9f*/0x0192,//LATIN SMALL LETTER F WITH HOOK
		/*0xa0*/0x00e1,//LATIN SMALL LETTER A WITH ACUTE
		/*0xa1*/0x00ed,//LATIN SMALL LETTER I WITH ACUTE
		/*0xa2*/0x00f3,//LATIN SMALL LETTER O WITH ACUTE
		/*0xa3*/0x00fa,//LATIN SMALL LETTER U WITH ACUTE
		/*0xa4*/0x00f1,//LATIN SMALL LETTER N WITH TILDE
		/*0xa5*/0x00d1,//LATIN CAPITAL LETTER N WITH TILDE
		/*0xa6*/0x00aa,//FEMININE ORDINAL INDICATOR
		/*0xa7*/0x00ba,//MASCULINE ORDINAL INDICATOR
		/*0xa8*/0x00bf,//INVERTED QUESTION MARK
		/*0xa9*/0x2310,//REVERSED NOT SIGN
		/*0xaa*/0x00ac,//NOT SIGN
		/*0xab*/0x00bd,//VULGAR FRACTION ONE HALF
		/*0xac*/0x00bc,//VULGAR FRACTION ONE QUARTER
		/*0xad*/0x00a1,//INVERTED EXCLAMATION MARK
		/*0xae*/0x00ab,//LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
		/*0xaf*/0x00bb,//RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
		/*0xb0*/0x2591,//LIGHT SHADE
		/*0xb1*/0x2592,//MEDIUM SHADE
		/*0xb2*/0x2593,//DARK SHADE
		/*0xb3*/0x2502,//BOX DRAWINGS LIGHT VERTICAL
		/*0xb4*/0x2524,//BOX DRAWINGS LIGHT VERTICAL AND LEFT
		/*0xb5*/0x2561,//BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
		/*0xb6*/0x2562,//BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
		/*0xb7*/0x2556,//BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
		/*0xb8*/0x2555,//BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
		/*0xb9*/0x2563,//BOX DRAWINGS DOUBLE VERTICAL AND LEFT
		/*0xba*/0x2551,//BOX DRAWINGS DOUBLE VERTICAL
		/*0xbb*/0x2557,//BOX DRAWINGS DOUBLE DOWN AND LEFT
		/*0xbc*/0x255d,//BOX DRAWINGS DOUBLE UP AND LEFT
		/*0xbd*/0x255c,//BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
		/*0xbe*/0x255b,//BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
		/*0xbf*/0x2510,//BOX DRAWINGS LIGHT DOWN AND LEFT
		/*0xc0*/0x2514,//BOX DRAWINGS LIGHT UP AND RIGHT
		/*0xc1*/0x2534,//BOX DRAWINGS LIGHT UP AND HORIZONTAL
		/*0xc2*/0x252c,//BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
		/*0xc3*/0x251c,//BOX DRAWINGS LIGHT VERTICAL AND RIGHT
		/*0xc4*/0x2500,//BOX DRAWINGS LIGHT HORIZONTAL
		/*0xc5*/0x253c,//BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
		/*0xc6*/0x255e,//BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
		/*0xc7*/0x255f,//BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
		/*0xc8*/0x255a,//BOX DRAWINGS DOUBLE UP AND RIGHT
		/*0xc9*/0x2554,//BOX DRAWINGS DOUBLE DOWN AND RIGHT
		/*0xca*/0x2569,//BOX DRAWINGS DOUBLE UP AND HORIZONTAL
		/*0xcb*/0x2566,//BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
		/*0xcc*/0x2560,//BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
		/*0xcd*/0x2550,//BOX DRAWINGS DOUBLE HORIZONTAL
		/*0xce*/0x256c,//BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
		/*0xcf*/0x2567,//BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
		/*0xd0*/0x2568,//BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
		/*0xd1*/0x2564,//BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
		/*0xd2*/0x2565,//BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
		/*0xd3*/0x2559,//BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
		/*0xd4*/0x2558,//BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
		/*0xd5*/0x2552,//BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
		/*0xd6*/0x2553,//BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
		/*0xd7*/0x256b,//BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
		/*0xd8*/0x256a,//BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
		/*0xd9*/0x2518,//BOX DRAWINGS LIGHT UP AND LEFT
		/*0xda*/0x250c,//BOX DRAWINGS LIGHT DOWN AND RIGHT
		/*0xdb*/0x2588,//FULL BLOCK
		/*0xdc*/0x2584,//LOWER HALF BLOCK
		/*0xdd*/0x258c,//LEFT HALF BLOCK
		/*0xde*/0x2590,//RIGHT HALF BLOCK
		/*0xdf*/0x2580,//UPPER HALF BLOCK
		/*0xe0*/0x03b1,//GREEK SMALL LETTER ALPHA
		/*0xe1*/0x00df,//LATIN SMALL LETTER SHARP S
		/*0xe2*/0x0393,//GREEK CAPITAL LETTER GAMMA
		/*0xe3*/0x03c0,//GREEK SMALL LETTER PI
		/*0xe4*/0x03a3,//GREEK CAPITAL LETTER SIGMA
		/*0xe5*/0x03c3,//GREEK SMALL LETTER SIGMA
		/*0xe6*/0x00b5,//MICRO SIGN
		/*0xe7*/0x03c4,//GREEK SMALL LETTER TAU
		/*0xe8*/0x03a6,//GREEK CAPITAL LETTER PHI
		/*0xe9*/0x0398,//GREEK CAPITAL LETTER THETA
		/*0xea*/0x03a9,//GREEK CAPITAL LETTER OMEGA
		/*0xeb*/0x03b4,//GREEK SMALL LETTER DELTA
		/*0xec*/0x221e,//INFINITY
		/*0xed*/0x03c6,//GREEK SMALL LETTER PHI
		/*0xee*/0x03b5,//GREEK SMALL LETTER EPSILON
		/*0xef*/0x2229,//INTERSECTION
		/*0xf0*/0x2261,//IDENTICAL TO
		/*0xf1*/0x00b1,//PLUS-MINUS SIGN
		/*0xf2*/0x2265,//GREATER-THAN OR EQUAL TO
		/*0xf3*/0x2264,//LESS-THAN OR EQUAL TO
		/*0xf4*/0x2320,//TOP HALF INTEGRAL
		/*0xf5*/0x2321,//BOTTOM HALF INTEGRAL
		/*0xf6*/0x00f7,//DIVISION SIGN
		/*0xf7*/0x2248,//ALMOST EQUAL TO
		/*0xf8*/0x00b0,//DEGREE SIGN
		/*0xf9*/0x2219,//BULLET OPERATOR
		/*0xfa*/0x00b7,//MIDDLE DOT
		/*0xfb*/0x221a,//SQUARE ROOT
		/*0xfc*/0x207f,//SUPERSCRIPT LATIN SMALL LETTER N
		/*0xfd*/0x00b2,//SUPERSCRIPT TWO
		/*0xfe*/0x25a0,//BLACK SQUARE
		/*0xff*/0x00a0//NO-BREAK SPACE
		};
		

		//    Name:     cp1252 to Unicode table
		//    Unicode version: 2.0    Table version: 2.01
		//    Table format:  Format A
		//    Date:          04/15/98
		//
		//    Contact:       Shawn.Steele@microsoft.com
		//
		//    General notes: none
		//
		//    Format: Three tab-separated columns
		//        Column #1 is the cp1252 code (in hex)
		//        Column #2 is the Unicode (in hex as 0xXXXX)
		//        Column #3 is the Unicode name (follows a comment sign, '#')
		//
		//    The entries are in cp1252 order
		//
		wchar_t cp1252[]=
		{
		/*0x00*/0x0000,//NULL
		/*0x01*/0x0000,// N/A 0x0001,//START OF HEADING
		/*0x02*/0x0000,// N/A 0x0002,//START OF TEXT
		/*0x03*/0x0000,// N/A 0x0003,//END OF TEXT
		/*0x04*/0x0000,// N/A 0x0004,//END OF TRANSMISSION
		/*0x05*/0x0000,// N/A 0x0005,//ENQUIRY
		/*0x06*/0x0000,// N/A 0x0006,//ACKNOWLEDGE
		/*0x07*/0x0000,// N/A 0x0007,//BELL
		/*0x08*/0x0000,// N/A 0x0008,//BACKSPACE
		/*0x09*/0x0000,// N/A 0x0009,//HORIZONTAL TABULATION
		/*0x0A*/0x0000,// N/A 0x000A,//LINE FEED
		/*0x0B*/0x0000,// N/A 0x000B,//VERTICAL TABULATION
		/*0x0C*/0x0000,// N/A 0x000C,//FORM FEED
		/*0x0D*/0x0000,// N/A 0x000D,//CARRIAGE RETURN
		/*0x0E*/0x0000,// N/A 0x000E,//SHIFT OUT
		/*0x0F*/0x0000,// N/A 0x000F,//SHIFT IN
		/*0x10*/0x0000,// N/A 0x0010,//DATA LINK ESCAPE
		/*0x11*/0x0000,// N/A 0x0011,//DEVICE CONTROL ONE
		/*0x12*/0x0000,// N/A 0x0012,//DEVICE CONTROL TWO
		/*0x13*/0x0000,// N/A 0x0013,//DEVICE CONTROL THREE
		/*0x14*/0x0000,// N/A 0x0014,//DEVICE CONTROL FOUR
		/*0x15*/0x0000,// N/A 0x0015,//NEGATIVE ACKNOWLEDGE
		/*0x16*/0x0000,// N/A 0x0016,//SYNCHRONOUS IDLE
		/*0x17*/0x0000,// N/A 0x0017,//END OF TRANSMISSION BLOCK
		/*0x18*/0x0000,// N/A 0x0018,//CANCEL
		/*0x19*/0x0000,// N/A 0x0019,//END OF MEDIUM
		/*0x1A*/0x0000,// N/A 0x001A,//SUBSTITUTE
		/*0x1B*/0x0000,// N/A 0x001B,//ESCAPE
		/*0x1C*/0x0000,// N/A 0x001C,//FILE SEPARATOR
		/*0x1D*/0x0000,// N/A 0x001D,//GROUP SEPARATOR
		/*0x1E*/0x0000,// N/A 0x001E,//RECORD SEPARATOR
		/*0x1F*/0x0000,// N/A 0x001F,//UNIT SEPARATOR
		/*0x20*/0x0020,//SPACE
		/*0x21*/0x0021,//EXCLAMATION MARK
		/*0x22*/0x0022,//QUOTATION MARK
		/*0x23*/0x0023,//NUMBER SIGN
		/*0x24*/0x0024,//DOLLAR SIGN
		/*0x25*/0x0025,//PERCENT SIGN
		/*0x26*/0x0026,//AMPERSAND
		/*0x27*/0x0027,//APOSTROPHE
		/*0x28*/0x0028,//LEFT PARENTHESIS
		/*0x29*/0x0029,//RIGHT PARENTHESIS
		/*0x2A*/0x002A,//ASTERISK
		/*0x2B*/0x002B,//PLUS SIGN
		/*0x2C*/0x002C,//COMMA
		/*0x2D*/0x002D,//HYPHEN-MINUS
		/*0x2E*/0x002E,//FULL STOP
		/*0x2F*/0x002F,//SOLIDUS
		/*0x30*/0x0030,//DIGIT ZERO
		/*0x31*/0x0031,//DIGIT ONE
		/*0x32*/0x0032,//DIGIT TWO
		/*0x33*/0x0033,//DIGIT THREE
		/*0x34*/0x0034,//DIGIT FOUR
		/*0x35*/0x0035,//DIGIT FIVE
		/*0x36*/0x0036,//DIGIT SIX
		/*0x37*/0x0037,//DIGIT SEVEN
		/*0x38*/0x0038,//DIGIT EIGHT
		/*0x39*/0x0039,//DIGIT NINE
		/*0x3A*/0x003A,//COLON
		/*0x3B*/0x003B,//SEMICOLON
		/*0x3C*/0x003C,//LESS-THAN SIGN
		/*0x3D*/0x003D,//EQUALS SIGN
		/*0x3E*/0x003E,//GREATER-THAN SIGN
		/*0x3F*/0x003F,//QUESTION MARK
		/*0x40*/0x0040,//COMMERCIAL AT
		/*0x41*/0x0041,//LATIN CAPITAL LETTER A
		/*0x42*/0x0042,//LATIN CAPITAL LETTER B
		/*0x43*/0x0043,//LATIN CAPITAL LETTER C
		/*0x44*/0x0044,//LATIN CAPITAL LETTER D
		/*0x45*/0x0045,//LATIN CAPITAL LETTER E
		/*0x46*/0x0046,//LATIN CAPITAL LETTER F
		/*0x47*/0x0047,//LATIN CAPITAL LETTER G
		/*0x48*/0x0048,//LATIN CAPITAL LETTER H
		/*0x49*/0x0049,//LATIN CAPITAL LETTER I
		/*0x4A*/0x004A,//LATIN CAPITAL LETTER J
		/*0x4B*/0x004B,//LATIN CAPITAL LETTER K
		/*0x4C*/0x004C,//LATIN CAPITAL LETTER L
		/*0x4D*/0x004D,//LATIN CAPITAL LETTER M
		/*0x4E*/0x004E,//LATIN CAPITAL LETTER N
		/*0x4F*/0x004F,//LATIN CAPITAL LETTER O
		/*0x50*/0x0050,//LATIN CAPITAL LETTER P
		/*0x51*/0x0051,//LATIN CAPITAL LETTER Q
		/*0x52*/0x0052,//LATIN CAPITAL LETTER R
		/*0x53*/0x0053,//LATIN CAPITAL LETTER S
		/*0x54*/0x0054,//LATIN CAPITAL LETTER T
		/*0x55*/0x0055,//LATIN CAPITAL LETTER U
		/*0x56*/0x0056,//LATIN CAPITAL LETTER V
		/*0x57*/0x0057,//LATIN CAPITAL LETTER W
		/*0x58*/0x0058,//LATIN CAPITAL LETTER X
		/*0x59*/0x0059,//LATIN CAPITAL LETTER Y
		/*0x5A*/0x005A,//LATIN CAPITAL LETTER Z
		/*0x5B*/0x005B,//LEFT SQUARE BRACKET
		/*0x5C*/0x005C,//REVERSE SOLIDUS
		/*0x5D*/0x005D,//RIGHT SQUARE BRACKET
		/*0x5E*/0x005E,//CIRCUMFLEX ACCENT
		/*0x5F*/0x005F,//LOW LINE
		/*0x60*/0x0060,//GRAVE ACCENT
		/*0x61*/0x0061,//LATIN SMALL LETTER A
		/*0x62*/0x0062,//LATIN SMALL LETTER B
		/*0x63*/0x0063,//LATIN SMALL LETTER C
		/*0x64*/0x0064,//LATIN SMALL LETTER D
		/*0x65*/0x0065,//LATIN SMALL LETTER E
		/*0x66*/0x0066,//LATIN SMALL LETTER F
		/*0x67*/0x0067,//LATIN SMALL LETTER G
		/*0x68*/0x0068,//LATIN SMALL LETTER H
		/*0x69*/0x0069,//LATIN SMALL LETTER I
		/*0x6A*/0x006A,//LATIN SMALL LETTER J
		/*0x6B*/0x006B,//LATIN SMALL LETTER K
		/*0x6C*/0x006C,//LATIN SMALL LETTER L
		/*0x6D*/0x006D,//LATIN SMALL LETTER M
		/*0x6E*/0x006E,//LATIN SMALL LETTER N
		/*0x6F*/0x006F,//LATIN SMALL LETTER O
		/*0x70*/0x0070,//LATIN SMALL LETTER P
		/*0x71*/0x0071,//LATIN SMALL LETTER Q
		/*0x72*/0x0072,//LATIN SMALL LETTER R
		/*0x73*/0x0073,//LATIN SMALL LETTER S
		/*0x74*/0x0074,//LATIN SMALL LETTER T
		/*0x75*/0x0075,//LATIN SMALL LETTER U
		/*0x76*/0x0076,//LATIN SMALL LETTER V
		/*0x77*/0x0077,//LATIN SMALL LETTER W
		/*0x78*/0x0078,//LATIN SMALL LETTER X
		/*0x79*/0x0079,//LATIN SMALL LETTER Y
		/*0x7A*/0x007A,//LATIN SMALL LETTER Z
		/*0x7B*/0x007B,//LEFT CURLY BRACKET
		/*0x7C*/0x007C,//VERTICAL LINE
		/*0x7D*/0x007D,//RIGHT CURLY BRACKET
		/*0x7E*/0x007E,//TILDE
		/*0x7F*/0x007F,//DELETE
		/*0x80*/0x20AC,//EURO SIGN
		/*0x81*/	0x0000,//UNDEFINED
		/*0x82*/0x201A,//SINGLE LOW-9 QUOTATION MARK
		/*0x83*/0x0192,//LATIN SMALL LETTER F WITH HOOK
		/*0x84*/0x201E,//DOUBLE LOW-9 QUOTATION MARK
		/*0x85*/0x2026,//HORIZONTAL ELLIPSIS
		/*0x86*/0x2020,//DAGGER
		/*0x87*/0x2021,//DOUBLE DAGGER
		/*0x88*/0x02C6,//MODIFIER LETTER CIRCUMFLEX ACCENT
		/*0x89*/0x2030,//PER MILLE SIGN
		/*0x8A*/0x0160,//LATIN CAPITAL LETTER S WITH CARON
		/*0x8B*/0x2039,//SINGLE LEFT-POINTING ANGLE QUOTATION MARK
		/*0x8C*/0x0152,//LATIN CAPITAL LIGATURE OE
		/*0x8D*/	0x0000,//UNDEFINED
		/*0x8E*/0x017D,//LATIN CAPITAL LETTER Z WITH CARON
		/*0x8F*/	0x0000,//UNDEFINED
		/*0x90*/	0x0000,//UNDEFINED
		/*0x91*/0x2018,//LEFT SINGLE QUOTATION MARK
		/*0x92*/0x2019,//RIGHT SINGLE QUOTATION MARK
		/*0x93*/0x201C,//LEFT DOUBLE QUOTATION MARK
		/*0x94*/0x201D,//RIGHT DOUBLE QUOTATION MARK
		/*0x95*/0x2022,//BULLET
		/*0x96*/0x2013,//EN DASH
		/*0x97*/0x2014,//EM DASH
		/*0x98*/0x02DC,//SMALL TILDE
		/*0x99*/0x2122,//TRADE MARK SIGN
		/*0x9A*/0x0161,//LATIN SMALL LETTER S WITH CARON
		/*0x9B*/0x203A,//SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
		/*0x9C*/0x0153,//LATIN SMALL LIGATURE OE
		/*0x9D*/	0x0000,//UNDEFINED
		/*0x9E*/0x017E,//LATIN SMALL LETTER Z WITH CARON
		/*0x9F*/0x0178,//LATIN CAPITAL LETTER Y WITH DIAERESIS
		/*0xA0*/0x00A0,//NO-BREAK SPACE
		/*0xA1*/0x00A1,//INVERTED EXCLAMATION MARK
		/*0xA2*/0x00A2,//CENT SIGN
		/*0xA3*/0x00A3,//POUND SIGN
		/*0xA4*/0x00A4,//CURRENCY SIGN
		/*0xA5*/0x00A5,//YEN SIGN
		/*0xA6*/0x00A6,//BROKEN BAR
		/*0xA7*/0x00A7,//SECTION SIGN
		/*0xA8*/0x00A8,//DIAERESIS
		/*0xA9*/0x00A9,//COPYRIGHT SIGN
		/*0xAA*/0x00AA,//FEMININE ORDINAL INDICATOR
		/*0xAB*/0x00AB,//LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
		/*0xAC*/0x00AC,//NOT SIGN
		/*0xAD*/0x00AD,//SOFT HYPHEN
		/*0xAE*/0x00AE,//REGISTERED SIGN
		/*0xAF*/0x00AF,//MACRON
		/*0xB0*/0x00B0,//DEGREE SIGN
		/*0xB1*/0x00B1,//PLUS-MINUS SIGN
		/*0xB2*/0x00B2,//SUPERSCRIPT TWO
		/*0xB3*/0x00B3,//SUPERSCRIPT THREE
		/*0xB4*/0x00B4,//ACUTE ACCENT
		/*0xB5*/0x00B5,//MICRO SIGN
		/*0xB6*/0x00B6,//PILCROW SIGN
		/*0xB7*/0x00B7,//MIDDLE DOT
		/*0xB8*/0x00B8,//CEDILLA
		/*0xB9*/0x00B9,//SUPERSCRIPT ONE
		/*0xBA*/0x00BA,//MASCULINE ORDINAL INDICATOR
		/*0xBB*/0x00BB,//RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
		/*0xBC*/0x00BC,//VULGAR FRACTION ONE QUARTER
		/*0xBD*/0x00BD,//VULGAR FRACTION ONE HALF
		/*0xBE*/0x00BE,//VULGAR FRACTION THREE QUARTERS
		/*0xBF*/0x00BF,//INVERTED QUESTION MARK
		/*0xC0*/0x00C0,//LATIN CAPITAL LETTER A WITH GRAVE
		/*0xC1*/0x00C1,//LATIN CAPITAL LETTER A WITH ACUTE
		/*0xC2*/0x00C2,//LATIN CAPITAL LETTER A WITH CIRCUMFLEX
		/*0xC3*/0x00C3,//LATIN CAPITAL LETTER A WITH TILDE
		/*0xC4*/0x00C4,//LATIN CAPITAL LETTER A WITH DIAERESIS
		/*0xC5*/0x00C5,//LATIN CAPITAL LETTER A WITH RING ABOVE
		/*0xC6*/0x00C6,//LATIN CAPITAL LETTER AE
		/*0xC7*/0x00C7,//LATIN CAPITAL LETTER C WITH CEDILLA
		/*0xC8*/0x00C8,//LATIN CAPITAL LETTER E WITH GRAVE
		/*0xC9*/0x00C9,//LATIN CAPITAL LETTER E WITH ACUTE
		/*0xCA*/0x00CA,//LATIN CAPITAL LETTER E WITH CIRCUMFLEX
		/*0xCB*/0x00CB,//LATIN CAPITAL LETTER E WITH DIAERESIS
		/*0xCC*/0x00CC,//LATIN CAPITAL LETTER I WITH GRAVE
		/*0xCD*/0x00CD,//LATIN CAPITAL LETTER I WITH ACUTE
		/*0xCE*/0x00CE,//LATIN CAPITAL LETTER I WITH CIRCUMFLEX
		/*0xCF*/0x00CF,//LATIN CAPITAL LETTER I WITH DIAERESIS
		/*0xD0*/0x00D0,//LATIN CAPITAL LETTER ETH
		/*0xD1*/0x00D1,//LATIN CAPITAL LETTER N WITH TILDE
		/*0xD2*/0x00D2,//LATIN CAPITAL LETTER O WITH GRAVE
		/*0xD3*/0x00D3,//LATIN CAPITAL LETTER O WITH ACUTE
		/*0xD4*/0x00D4,//LATIN CAPITAL LETTER O WITH CIRCUMFLEX
		/*0xD5*/0x00D5,//LATIN CAPITAL LETTER O WITH TILDE
		/*0xD6*/0x00D6,//LATIN CAPITAL LETTER O WITH DIAERESIS
		/*0xD7*/0x00D7,//MULTIPLICATION SIGN
		/*0xD8*/0x00D8,//LATIN CAPITAL LETTER O WITH STROKE
		/*0xD9*/0x00D9,//LATIN CAPITAL LETTER U WITH GRAVE
		/*0xDA*/0x00DA,//LATIN CAPITAL LETTER U WITH ACUTE
		/*0xDB*/0x00DB,//LATIN CAPITAL LETTER U WITH CIRCUMFLEX
		/*0xDC*/0x00DC,//LATIN CAPITAL LETTER U WITH DIAERESIS
		/*0xDD*/0x00DD,//LATIN CAPITAL LETTER Y WITH ACUTE
		/*0xDE*/0x00DE,//LATIN CAPITAL LETTER THORN
		/*0xDF*/0x00DF,//LATIN SMALL LETTER SHARP S
		/*0xE0*/0x00E0,//LATIN SMALL LETTER A WITH GRAVE
		/*0xE1*/0x00E1,//LATIN SMALL LETTER A WITH ACUTE
		/*0xE2*/0x00E2,//LATIN SMALL LETTER A WITH CIRCUMFLEX
		/*0xE3*/0x00E3,//LATIN SMALL LETTER A WITH TILDE
		/*0xE4*/0x00E4,//LATIN SMALL LETTER A WITH DIAERESIS
		/*0xE5*/0x00E5,//LATIN SMALL LETTER A WITH RING ABOVE
		/*0xE6*/0x00E6,//LATIN SMALL LETTER AE
		/*0xE7*/0x00E7,//LATIN SMALL LETTER C WITH CEDILLA
		/*0xE8*/0x00E8,//LATIN SMALL LETTER E WITH GRAVE
		/*0xE9*/0x00E9,//LATIN SMALL LETTER E WITH ACUTE
		/*0xEA*/0x00EA,//LATIN SMALL LETTER E WITH CIRCUMFLEX
		/*0xEB*/0x00EB,//LATIN SMALL LETTER E WITH DIAERESIS
		/*0xEC*/0x00EC,//LATIN SMALL LETTER I WITH GRAVE
		/*0xED*/0x00ED,//LATIN SMALL LETTER I WITH ACUTE
		/*0xEE*/0x00EE,//LATIN SMALL LETTER I WITH CIRCUMFLEX
		/*0xEF*/0x00EF,//LATIN SMALL LETTER I WITH DIAERESIS
		/*0xF0*/0x00F0,//LATIN SMALL LETTER ETH
		/*0xF1*/0x00F1,//LATIN SMALL LETTER N WITH TILDE
		/*0xF2*/0x00F2,//LATIN SMALL LETTER O WITH GRAVE
		/*0xF3*/0x00F3,//LATIN SMALL LETTER O WITH ACUTE
		/*0xF4*/0x00F4,//LATIN SMALL LETTER O WITH CIRCUMFLEX
		/*0xF5*/0x00F5,//LATIN SMALL LETTER O WITH TILDE
		/*0xF6*/0x00F6,//LATIN SMALL LETTER O WITH DIAERESIS
		/*0xF7*/0x00F7,//DIVISION SIGN
		/*0xF8*/0x00F8,//LATIN SMALL LETTER O WITH STROKE
		/*0xF9*/0x00F9,//LATIN SMALL LETTER U WITH GRAVE
		/*0xFA*/0x00FA,//LATIN SMALL LETTER U WITH ACUTE
		/*0xFB*/0x00FB,//LATIN SMALL LETTER U WITH CIRCUMFLEX
		/*0xFC*/0x00FC,//LATIN SMALL LETTER U WITH DIAERESIS
		/*0xFD*/0x00FD,//LATIN SMALL LETTER Y WITH ACUTE
		/*0xFE*/0x00FE,//LATIN SMALL LETTER THORN
		/*0xFF*/0x00FF,//LATIN SMALL LETTER Y WITH DIAERESIS
		};
	}
}

