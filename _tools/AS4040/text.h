
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                                Text String                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
// (c) 2020-2024 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com

// multithreaded LONG increment
inline void LongIncrement(long* num)
{
//#ifdef _MT
//	::InterlockedIncrement(num);
//#else // _MT
	*num = *num + 1;
//#endif // _MT
}

// multithreaded LONG decrement (returns new state)
inline long LongDecrement(long* num)
{
//#ifdef _MT
//	return ::InterlockedDecrement(num);
//#else // _MT
	long res = *num - 1;
	*num = res;
	return res;
//#endif // _MT
}

///////////////////////////////////////////////////////////////////////////////
// inplace "new" operator
//		example:	new (&m_List[inx]) CText;
//					m_List[i].~CText()
/*
//inline void* operator new (long unsigned int size, void* p)
inline void* operator new (size_t size, void* p)
{
	return p;
}

inline void operator delete (void* adr, void* p)
{
	return;
}

inline int sqr(int n) { return n*n; }
*/
///////////////////////////////////////////////////////////////////////////////
// Text string data (8 bytes + data)

typedef struct TEXTDATA_
{
	// reference counter (number of owners, 0=nobody)
	long		ref;

	// length of text string
	long		length;

	// text string data (including terminating zero)
	TCHAR		data[1];

} TEXTDATA;

// size of text string data head
#define SIZEOFTEXTDATA (2*sizeof(long))

// empty text string data
extern TEXTDATA EmptyTextData;

// convert text to INT number
int Int(LPCTSTR text);

// convert text to HEX number
DWORD Hex(LPCTSTR text);

// convert text to DOUBLE number
double Double(LPCTSTR text);

///////////////////////////////////////////////////////////////////////////////
// Text string

class CText
{
protected:

	// pointer to text string data
	TEXTDATA*	m_Data;

	// create new text data (old text data must be delete)
	// (on error attaches empty text string data and returns FALSE)
	BOOL NewBuffer(int length);

	// resize data buffer (ref must be = 1, returns FALSE on error)
	BOOL SetSize(int length);

public:

	// constructors
	inline CText() { Attach(&EmptyTextData); }
	inline CText(TEXTDATA* data) { Attach(data); }
	inline CText(const CText& src) { Attach(src.m_Data); }
	CText(TCHAR chr);
	CText(LPCTSTR txt, int length = -1);
	CText(CText txt1, CText txt2);
	CText(LPCTSTR txt1, CText txt2);
	CText(CText txt1, LPCTSTR txt2);
	CText(TCHAR chr, CText txt);
	CText(CText txt, TCHAR chr);

	// destructor
	inline ~CText() { Detach(); }

	// attach text string data
	void Attach(TEXTDATA* data);

	// detach (and destroy) text string data
	void Detach();

	// detach and attach text string data (avoid deleting identical data)
	inline void DetachAttach(TEXTDATA* data)
	{
		if (data != m_Data)
		{
			Detach();
			Attach(data);
		}
	}

	// appropriate text string data before write (returns FALSE on error)
	BOOL CopyWrite();

	// get reference counter of text string data
	inline int Ref() const { return m_Data->ref; }

	// text length
	inline int Length() const { return m_Data->length; }
	BOOL SetLength(int length);

	// load Unicode character from UTF8 string (shifts character index)
	DWORD GetUni(int* index) const;

	// check if string is empty
	inline BOOL IsEmpty() const { return m_Data->length == 0; }
	inline BOOL IsNotEmpty() const { return m_Data->length != 0; }

	// get text data
	inline TEXTDATA* Data() const { return m_Data; }

	// get text string
	inline TCHAR* DataData() const { return m_Data->data; }

	// retype operator
	inline operator LPCTSTR() const { return m_Data->data; }

	// empty string
	inline void Empty() { Detach(); Attach(&EmptyTextData); }

	// check if character index is valid
	inline BOOL IsValid(int inx) const
		{ return (DWORD)inx < (DWORD)m_Data->length; }
	inline BOOL IsNotValid(int inx) const
		{ return (DWORD)inx >= (DWORD)m_Data->length; }

	// array operator (without index checking)
	inline const TCHAR& operator[] (int inx) const
		{ return m_Data->data[inx]; }

	// get character at given position (with index checking)
	inline TCHAR At(int inx) const { if (IsValid(inx))
			return m_Data->data[inx]; else return (TCHAR)'\0'; }
	inline TCHAR First() const { return At(0); }
	inline TCHAR Last() const { return At(m_Data->length-1); }

	// set character (with index checking, returns FALSE on error)
	BOOL SetAt(int inx, TCHAR chr);
	inline BOOL SetFirst(TCHAR chr) { return SetAt(0, chr); }
	inline BOOL SetLast(TCHAR chr) { return SetAt(m_Data->length-1, chr); }

	// set text (returns FALSE on error)
	void Set(const CText& txt);
	BOOL Set(LPCTSTR txt, int length = -1);
	BOOL Set(TCHAR chr);

	// add repeated text (returns FALSE on error)
	BOOL AddRep(int count, CText txt);
	BOOL AddRep(int count, LPCTSTR txt, int length = -1);
	BOOL AddRep(int count, TCHAR chr);

	// add text (returns FALSE on error)
	inline BOOL Add(CText txt) { return AddRep(1, txt); }
	inline BOOL Add(LPCTSTR txt, int length = -1)
		{ return AddRep(1, txt, length); }
	inline BOOL Add(TCHAR chr) { return AddRep(1, chr); }

	// write text into buffer (without terminating zero)
	void Write(TCHAR* buf, int maxlen = BIGINT) const;

	// write text into buffer with terminating zero
	void WriteZ(TCHAR* buf, int maxlen = BIGINT) const;

	// find character (-1=not found)
	int Find(TCHAR chr, int pos = 0) const;
	int FindRev(TCHAR chr, int pos = BIGINT) const;

	// find white space character (space, tab, newline, -1 = not found)
	int FindSpace(int pos = 0) const;
	int FindSpaceRev(int pos = BIGINT) const;

	// find non white space character (no space, tab, newline, -1 = not found)
	int FindNSpace(int pos = 0) const;
	int FindNSpaceRev(int pos = BIGINT) const;

	// find text (-1 = not found)
	int Find(const CText& txt, int pos = 0) const;
	int FindRev(const CText& txt, int pos = BIGINT) const;
	int Find(LPCTSTR txt, int length = -1, int pos = 0) const;
	int FindRev(LPCTSTR txt, int length = -1, int pos = BIGINT) const;

	// find characters from the list (-1 = not found)
	int FindList(LPCTSTR list, int count = -1, int pos = 0) const;
	inline int FindList(const CText& list, int pos = 0) const
		{ return FindList(list.m_Data->data, list.m_Data->length, pos); }
	int FindListRev(LPCTSTR list, int count = -1, int pos = BIGINT) const;
	inline int FindListRev(const CText& list, int pos = BIGINT) const
		{ return FindListRev(list.m_Data->data, list.m_Data->length, pos); }

	// find characters except the list (-1 = not found)
	int FindExcept(LPCTSTR list, int count = -1, int pos = 0) const;
	inline int FindExcept(const CText& list, int pos = 0) const
		{ return FindExcept(list.m_Data->data, list.m_Data->length, pos); }
	int FindExceptRev(LPCTSTR list, int count = -1, int pos = BIGINT) const;
	inline int FindExceptRev(const CText& list, int pos = BIGINT) const
		{ return FindExceptRev(list.m_Data->data, list.m_Data->length, pos); }

	// substitute strings (returns FALSE on error)
	BOOL Subst(CText txt, CText subst);

	// substitute characters from the list (returns FALSE on error)
	BOOL SubstList(LPCTSTR list, LPCTSTR subst, int count = -1);
	BOOL SubstList(CText list, CText subst);

	// convert text to uppercase (returns FALSE on error)
	BOOL UpperCase();

	// convert text to lowercase (returns FALSE on error)
	BOOL LowerCase();

	// left part of text (returns empty string on error)
	CText Left(int len) const;

	// right part of text (returns empty string on error)
	CText Right(int len) const;
	CText RightFrom(int pos) const;

	// middle part of text (returns empty string on error)
	CText Mid(int pos, int len) const;

	// insert text (returns FALSE on error)
	BOOL Insert(int pos, LPCTSTR txt, int length = -1);
	inline BOOL Insert(int pos, CText txt)
		{ return Insert(pos, txt.m_Data->data, txt.m_Data->length); }
	BOOL Insert(int pos, TCHAR chr);

	// delete characters (returns FALSE on error)
	BOOL Delete(int pos, int count = 1);
	inline BOOL DelFirst() { return Delete(0); }
	inline BOOL DelLast() { return Delete(m_Data->length - 1); }
	inline BOOL DelToEnd(int pos) { return Delete(pos, BIGINT); }

	// delete characters from the list (returns FALSE on error)
	BOOL DelList(LPCTSTR list, int count = -1);
	inline BOOL DelList(CText list)
		{ return DelList(list.m_Data->data, list.m_Data->length); }

	// delete characters except the list (returns FALSE on error)
	BOOL DelExcept(LPCTSTR list, int count = -1);
	inline BOOL DelExcept(CText list)
		{ return DelExcept(list.m_Data->data, list.m_Data->length); }

	// find and delete all words (return FALSE on error)
	BOOL DelWord(LPCTSTR txt, int length = -1);
	inline BOOL DelWord(CText txt)
		{ return DelWord(txt.m_Data->data, txt.m_Data->length); }

	// trim white spaces from the string (returns FALSE on error)
	BOOL TrimLeft();
	BOOL TrimRight();
	inline BOOL Trim() { return TrimRight() && TrimLeft(); }

	// nul correction (check and truncate text on NUL character)
	void CheckNul();

	// get number of rows using CR/LF (or LF)
	int RowsNum() const;
	int RowsNumLF() const;

	// get one row using CR/LF (or LF, trim CR))
	CText Row(int row) const;
	CText RowLF(int row) const;

	// convert INT number to text (returns FALSE on error)
	BOOL SetInt(int num, BOOL add = FALSE);
	inline BOOL AddInt(int num) { return SetInt(num, TRUE); }

	// set 2 digits (returns FALSE on error)
	BOOL Set2Dig(int num, BOOL add = FALSE);
	inline BOOL Add2Dig(int num) { return Set2Dig(num, TRUE); }

	// set 2 digits with space padding (returns FALSE on error)
	BOOL Set2DigSpc(int num, BOOL add = FALSE);
	inline BOOL Add2DigSpc(int num) { return Set2DigSpc(num, TRUE); }

	// set 4 digits (returns FALSE on error)
	BOOL Set4Dig(int num, BOOL add = FALSE);
	inline BOOL Add4Dig(int num) { return Set4Dig(num, TRUE); }

	// convert DWORD number to text as HEX (return FALSE on error)
	BOOL SetHex(DWORD num, int digits = -1, BOOL add = FALSE);
	inline BOOL AddHex(DWORD num, int digits = -1)
		{ return SetHex(num, digits, TRUE); }

	// convert DOUBLE number to text (returns FALSE on error)
	BOOL SetDouble(double num, int digits = 15, BOOL add = FALSE);
	inline BOOL AddDouble(double num, int digits = 15)
		{ return SetDouble(num, digits, TRUE); }
	inline BOOL SetFloat(float num, int digits = 7, BOOL add = FALSE)
		{ return SetDouble(num, digits, add); }
	inline BOOL AddFloat(float num, int digits = 7)
		{ return SetDouble(num, digits, TRUE); }

	// format string (using printf() format, returns FALSE on error)
	BOOL Format(LPCTSTR format, ...);
	BOOL FormatV(LPCTSTR format, va_list args);
	BOOL AddFormat(LPCTSTR format, ...);
	BOOL AddFormatV(LPCTSTR format, va_list args);

	// convert text to INT/SHORT/CHAR number
	inline int Int() const { return ::Int(m_Data->data); }
	inline short Short() const { return (short)Int(); }
	inline char Char() const { return (char)Int(); }

	// convert text to HEX/WORD/BYTE number
	inline DWORD Hex() const { return ::Hex(m_Data->data); }
	inline WORD Word() const { return (WORD)Hex(); }
	inline BYTE Byte() const { return (BYTE)Hex(); }

	// convert text to DOUBLE/FLOAT number
	inline double Double() const { return ::Double(m_Data->data); };
	inline float Float() const { return (float)Double(); };

	// load/save file (returns FALSE on error)
	BOOL LoadFile(FILE* file, int size);
	BOOL LoadFileName(LPCTSTR name);
	BOOL SaveFile(FILE* file) const;
	BOOL SaveFileName(LPCTSTR name) const;

	// set operators
	const CText& operator= (const CText& txt);
	const CText& operator= (LPCTSTR txt);
	const CText& operator= (TCHAR chr);

	// add operators
	const CText& operator+= (CText txt);
	const CText& operator+= (LPCTSTR txt);
	const CText& operator+= (TCHAR chr);
	inline friend CText operator+ (const CText& txt1, const CText& txt2)
		{ return CText(txt1, txt2); }
	inline friend CText operator+ (LPCTSTR txt1, const CText& txt2)
		{ return CText(txt1, txt2); }
	inline friend CText operator+ (const CText& txt1, LPCTSTR txt2)
		{ return CText(txt1, txt2); }
	inline friend CText operator+ (TCHAR chr, const CText& txt)
		{ return CText(chr, txt); }
	inline friend CText operator+ (const CText& txt, TCHAR chr)
		{ return CText(txt, chr); }

	// compare operators
	friend BOOL operator== (const CText& txt1, const CText& txt2);
	friend BOOL operator== (LPCTSTR txt1, const CText& txt2);
	friend BOOL operator== (const CText& txt1, LPCTSTR txt2);
	friend BOOL operator== (TCHAR chr, const CText& txt);
	friend BOOL operator== (const CText& txt, TCHAR chr);
	friend BOOL operator!= (const CText& txt1, const CText& txt2);
	friend BOOL operator!= (LPCTSTR txt1, const CText& txt2);
	friend BOOL operator!= (const CText& txt1, LPCTSTR txt2);
	friend BOOL operator!= (TCHAR chr, const CText& txt);
	friend BOOL operator!= (const CText& txt, TCHAR chr);
};

extern CText EmptyText;

extern CText NewLine;

///////////////////////////////////////////////////////////////////////////////
// text string list

class CTextList
{
protected:

	// number of text strings in the list
	int		m_Num;

	// max. number of text strings (size of text array)
	int		m_Max;

	// array of text strings
	CText*	m_List;

	// set array size (without initializing entries, returns FALSE on error)
	BOOL SetSize(int num);

public:

	// constructor
	inline CTextList() { m_Num = 0; m_Max = 0; m_List = NULL; }

	// destructor
	~CTextList();

	// number of text strings in the list
	inline int Num() const { return m_Num; }

	// array of text strings
	inline const CText* List() const { return m_List; }
	inline CText* List() { return m_List; }

	// delete all entries
	void DelAll();

	// check if index is valid
	inline BOOL IsValid(int inx) const { return (DWORD)inx < (DWORD)m_Num; }
	inline BOOL IsNotValid(int inx) const { return (DWORD)inx>=(DWORD)m_Num; }

	// array operator (without index checking)
	inline CText& operator[] (int inx) { return m_List[inx]; }
	inline const CText& operator[] (int inx) const { return m_List[inx]; }

	// get text entry (with index checking)
	inline CText Get(int inx)
		{ if (IsValid(inx)) return m_List[inx]; else return EmptyText; }

	// set text entry (with index checking)
	inline void Set(int inx, const CText txt)
		{ if (IsValid(inx)) m_List[inx] = txt; }

	// add new empty text entry (returns index or -1 on error)
	int New();

	// add new text entry (returns index or -1 on error)
	int Add(const CText& txt);

	// add another list (returns index of first entry or -1 on error)
	int Add(const CTextList* list);

	// insert text entry into list (limit index range, returns FALSE on error)
	BOOL Insert(int inx, const CText& txt);

	// insert another list (limit index range, returns FALSE on error)
	BOOL Insert(int inx, const CTextList* list);

	// delete entry from the list (check index range)
	void Delete(int inx, int num = 1);
	inline void DelFirst() { Delete(0); }
	inline void DelLast() { Delete(BIGINT); }
	inline void DelToEnd(int inx) { Delete(inx, BIGINT); }

	// duplicate entry (returns index or -1 on error)
	int Dup(int inx);

	// copy list from another list(destroys old content,returns FALSE on error)
	BOOL Copy(const CTextList* list);

	// operators
	inline const CTextList& operator= (const CTextList& src)
		{ Copy(&src); return *this; }
	inline const CTextList& operator+= (const CText& src)
		{ Add(src); return *this; }
	inline const CTextList& operator+= (const CTextList& src)
		{ Add(&src); return *this; }

	// split text string (destroys old content, returns FALSE on error)
	BOOL Split(const CText& text,LPCTSTR delim,int length=-1,int limit=BIGINT);
	BOOL Split(const CText& text, TCHAR delim = (TCHAR)' ', int limit = BIGINT);
	inline BOOL Split(const CText& text, const CText& delim, int limit=BIGINT)
		{ return Split(text, delim.DataData(), delim.Length(), limit); }
	BOOL SplitWords(const CText& text, int limit = BIGINT);
	BOOL SplitLines(const CText& text, int limit = BIGINT);

	// join text strings
	CText Join(LPCTSTR delim, int length = -1) const;
	CText Join(TCHAR delim = (TCHAR)' ') const;
	CText JoinLines() const;
	inline CText Join(const CText& delim) const
		{ return Join(delim.DataData(), delim.Length()); }
};
