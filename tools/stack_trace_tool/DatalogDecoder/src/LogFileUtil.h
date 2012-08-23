/*****************************************************************************\
**
** LogFileUtil.h
**
** File utility class.
**
\*****************************************************************************/

#ifndef LOGFILEUTIL_H
#define LOGFILEUTIL_H

/* Includes ******************************************************************/

#include <zlib.h>

namespace DECODER {

/* Forward Declarations ******************************************************/

/* Constants *****************************************************************/

enum StreamArg_Type {
	StreamArg_Invalid = -1,
	StreamArg_SignedChar = 1,
	StreamArg_UnsignedChar,
	StreamArg_SignedInt,
	StreamArg_UnsignedInt,
	StreamArg_SignedLong,
	StreamArg_UnsignedLong,
	StreamArg_String,
	StreamArg_Float,
	StreamArg_Double,
	StreamArg_Bool,
	StreamArg_Flag = 100,
	StreamArg_Precision
};

enum PrintArg_Type {
	PrintArg_Invalid = -1,
	PrintArg_Char = 1,
	PrintArg_Int,
	PrintArg_Long,
	PrintArg_Float,
	PrintArg_Double,
	PrintArg_String,
	PrintArg_Done,
	PrintArg_Error
};

enum StreamArg_Flag {
	FLAG_showpos = 0x0001,		// print explicit '+' on output of positive values
	FLAG_dec = 0x0010,			// output integers as decimal values
	FLAG_hex = 0x0020,			// output integers as hexadecimal values
	FLAG_oct = 0x0040,			// output integers as octal values
	FLAG_basemask = 0x0070,		// mask for output base flags
	FLAG_showbase = 0x0100,		// show integer output base (e.g. 0x for hex)
	FLAG_showpoint = 0x1000,	// show trailing zeroes after decimal point
	FLAG_scientific = 0x2000,	// scientific notation (e.g. 1.234E02)
	FLAG_fixed = 0x4000,			// fixed point notation (e.g. 123.4)
	FLAG_floatmask = 0x6000,	// mask for floating point format flags
};

/* Globals *******************************************************************/

/* Macros ********************************************************************/

/* Prototypes ****************************************************************/

/* Classes *******************************************************************/

/* Functions *****************************************************************/

static int egzread(gzFile file, void * buffer, size_t size)
{
	unsigned char *cBuffer = (unsigned char *)buffer;
	int retVal = gzread(file, buffer, size);

	if (retVal != Z_ERRNO && retVal != 0) {
		for (size_t i = 0; i < size; ++i, ++cBuffer) {
			*cBuffer ^= 0xa5;
		}
	}
	else {
		std::memset(buffer, 0, size);
	}

	return retVal;
}

static int egzgetc(gzFile file)
{
	int ch = gzgetc(file);

	return ch != Z_ERRNO ? ch ^ 0xa5 : 0;
}

static void egzreadstr(gzFile file, std::string &string, int length) 
{
	char *buffer = new char[length + 1];

	egzread(file, buffer, length);
	buffer[length] = '\0';
	string = buffer;

	delete [] buffer;
}

static void egzgetstr(gzFile file, std::string &string, int length) 
{
	for (int i = 0; i < length; ++i) {
		string.push_back(egzgetc(file));
	}
}

static void fgetstr(FILE * file, std::string &string, int length) 
{
	for (int i = 0; i < length; ++i) {
		string.push_back(fgetc(file));
	}
}

static const char *GetNextArg(const Char *format, PrintArg_Type &arg)
{
	enum ParseState {
		kWaitForPercent,
		kWaitForType
	};

	if (format == NULL) {
		arg = PrintArg_Error;
		return NULL;
	}

	if (*format == '\0') {
		arg = PrintArg_Done;
		return NULL;
	}

	ParseState parseState = kWaitForPercent;
	Bool longModifierPresent = false;
	Bool parseDone = false;

	arg = PrintArg_Done;
	while (*format != '\0' && !parseDone) {
		if ((parseState == kWaitForPercent) && (*format == '%')) {
			parseState = kWaitForType;
		}
		else if (parseState == kWaitForType) {
			switch (*format) {
			case '%':
				if (*(format - 1) == '%') {
					// Combination of %% corresponds to single % output.
					parseState = kWaitForPercent;
				}
				else {
					// The format string contains an error.
					arg = PrintArg_Error;
					parseDone = true;
				}
				break;
			case '*':
				break;
			case 'l':
				longModifierPresent = true;
				break;
			case 'c':
				arg = PrintArg_Char;
				parseDone = true;
				break;
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				arg = (longModifierPresent) ? PrintArg_Long : PrintArg_Int;
				parseDone = true;
				break;
			case 'f':
			case 'g':
				arg = (longModifierPresent) ? PrintArg_Double : PrintArg_Float;
				parseDone = true;
				break;
			case 's':
				arg = PrintArg_String;
				parseDone = true;
				break;
			}
		}

		++format;
	}

	if ((*format == '\0') && (arg == PrintArg_Done) &&  (parseState == kWaitForType)) {
		// Reached end of format string with incomplete format specifier
		arg = PrintArg_Error;
	}

	return format;
}


static Uint16 ConvertToSTL(Uint16 flags)
{
	Uint16 value = 0;

	value |= flags & FLAG_showpos ? std::ios::showpos : 0;
	value |= flags & FLAG_showbase ? std::ios::showbase : 0;
	value |= flags & FLAG_showpoint ? std::ios::showpoint : 0;

	value |= flags & FLAG_dec ? std::ios::dec : 0;
	value |= flags & FLAG_hex ? std::ios::hex : 0;
	value |= flags & FLAG_oct ? std::ios::oct : 0;
			
	value |= flags & FLAG_scientific ? std::ios::scientific : 0;
	value |= flags & FLAG_fixed ? std::ios::fixed : 0;

	value |= (flags & FLAG_showpoint) && !(flags & FLAG_scientific)? std::ios::fixed : 0;

	return value;
}

template <typename _TYPE>
inline _TYPE egzget(gzFile file) 
{
	_TYPE value = 0;

	egzread(file, &value, sizeof(value));

	return value;
}

template <typename _TYPE>
static void egzgetformatstr(gzFile file, const char *format, int len, std::string &string)
{
	_TYPE value = 0;
	String formatStr(format, len);
	Int32 index = formatStr.find('%');
	Char buffer[256] = { 0 }; // Bump it up if needed.

	index = formatStr.find('*', index);
	while (index != std::string::npos) {
		sprintf(buffer, "%d", egzget<Int32>(file));
		formatStr.replace(index, 1, buffer);
	}

	egzread(file, &value, sizeof(value));
	sprintf(buffer, formatStr.c_str(), value);

	string = buffer;
}

template <>
static void egzgetformatstr<String>(gzFile file, const char *format, int len, std::string &string)
{
	String formatStr(format, len);
	Int32 index = formatStr.find('%');
	Uint16 stringLen = 0;

	string.append(format, index);

	egzread(file, &stringLen, sizeof(stringLen));
	egzgetstr(file, string, stringLen);
}

static const char* egzgetformatstr(gzFile file, const char *format, std::string &string)
{
	const Char *index = format;
	const Char *end = format;
	PrintArg_Type argType = PrintArg_Invalid;

	string.clear();

	end = GetNextArg(format, argType);

	if (argType != PrintArg_Error) {
		switch (argType) {
		case PrintArg_Char:
			egzgetformatstr<Char>(file, index, end - index, string);
			break;
		case PrintArg_Int:
		case PrintArg_Long:
			egzgetformatstr<Int32>(file, index, end - index, string);
			break;
		case PrintArg_Float:
			egzgetformatstr<Float>(file, index, end - index, string);
			break;
		case PrintArg_Double:
			egzgetformatstr<Double>(file, index, end - index, string);
			break;
		case PrintArg_String:
			egzgetformatstr<String>(file, index, end - index, string);
			break;
		case PrintArg_Done:
			string += format;
			break;
		}
	}

	return end;
}

} // namespace DECODER

#endif // LOGFILEUTIL_H

