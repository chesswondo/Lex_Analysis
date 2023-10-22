#define  _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// ��� ����������� ������
enum Color {
    BLACK             = 0,
    DARKBLUE          = FOREGROUND_BLUE,
    DARKGREEN         = FOREGROUND_GREEN,
    DARKCYAN          = FOREGROUND_GREEN | FOREGROUND_BLUE,
    DARKRED           = FOREGROUND_RED,
    DARKMAGENTA       = FOREGROUND_RED | FOREGROUND_BLUE,
    DARKYELLOW        = FOREGROUND_RED | FOREGROUND_GREEN,
    DARKGRAY          = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    GRAY              = FOREGROUND_INTENSITY,
    BLUE              = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    GREEN             = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    CYAN              = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    RED               = FOREGROUND_INTENSITY | FOREGROUND_RED,
    MAGENTA           = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    YELLOW            = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    WHITE             = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};
void out(char c, int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
    printf("%c",c);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)GRAY);
}
void outs(const char * s, int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
    printf("%s",s);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)GRAY);
}

// ��������� ��� ���������� ��������� ����� (��� ��������; ������ ������� C++)
typedef struct Vector_
{
    unsigned int size, count;
    const char ** data;
} Vector;
// ��������� �������
Vector * makeVec()
{
    Vector * v = malloc(sizeof(Vector));
    if (v == NULL) exit(1);
    v->size  = 4;  // Initial size
    v->count = 0;
    v->data = malloc(sizeof(const char*)*v->size);
    if (v->data == NULL) exit(1);
    return v;
}
// ��������� ���'��
void killVec(Vector* v)
{
    for(unsigned int i = 0; i < v->count; ++i)
        free((void*)v->data[i]);
    free(v->data);
    free(v);
}
// ��������� �����
void add(Vector*v, const char * elem)
{
    for(unsigned int i = 0; i < v->count; ++i)
    {
        if (_stricmp(v->data[i],elem) == 0) return;  // ���� ����� ��� � � �� ��������
    }
    if (v->count == v->size)                       // �������� ���� ���'��
    {
        v->data = realloc(v->data,(v->size*=2)*sizeof(const char*));
        if (v->data == NULL) exit(1);
    }
    v->data[v->count++] = _strdup(elem);           // ��������� ���������� �����
}
// ��������� �������� �������
void outv(Vector*v)
{
    for(unsigned int i = 0; i < v->count; ++i)
        printf("\t%s\n",v->data[i]);
}

Vector   // ������� ��� ����� �����
    *Numbers,       *Strings,       *Comments,
    *Directives,    *Instructions,  *Registers,
    *Reserved,      *Identifiers,   *Operators,
    *Delimeters,    *Unknowns;
// ���������� (���� �������� �������...)
const char * instructions[] =
{
    "mov",  "lea",  "add",  "adc",  "sub",  "sbb",  "imul",  "mul",  "idiv", "div", 
    "inc",  "dec",  "neg",  "cmp",  "daa",  "das",  "aaa",   "aas",  "aam",  "aad",
    "and",  "or",   "xor",  "not",  "sar",  "shr",  "sal",   "shl",  "shrd", "shld",
    "ror",  "rol",  "rcr",  "rcl",  "jmp",  "je",   "jz",    "jne",  "jnz",  "jbe", 
    "repe", "loop", "scasb","stosb","jnc",  "lodsb","call",  "ret",  "iret", "int", 
    "into", "bound","enter","leave","push", "pop",  "pusha", "popa", "inc",  "dec", 
    "jc",   "test",
    // and many, many, many others
};
// ������� (�� ��...)
const char * registers[] =
{
    "al",  "ah",  "ax",  "eax",
    "bl",  "bh",  "bx",  "ebx",
    "cl",  "ch",  "cx",  "ecx",
    "dl",  "dh",  "dx",  "edx",
    "cs",  "ds",  "es",  "fs",   "ss",  "gs",
    "bp",  "ebp", "di",  "edi",  "si",  "esi",  "sp",  "esp",
    // and many, many, many others
};
// ������������ �����
const char * reserved[] =
{
    "segment",  "byte",     "word",     "dword",    "db",   "dw",
    "dd",       "ptr",      "equ",      "proc",     "near", "far", 
    "endp",     "use16",    "extrn",    "dup",
    // and many, many, many others
};
// ���������
const char * directives[] =
{
    "assume", "locals", "org", "segment", "ends", "end",
    // and many, many, many others
};

// ������ ��� ���������� ������ ������ (������� ��������)
#define SIZE(array)  sizeof(array)/sizeof((array)[0])
// ����� �������� � ������
int findIn(const char * name, const char ** array, unsigned int count)
{
    for(unsigned int i = 0; i < count; ++i)
        if (_stricmp(array[i],name) == 0) return 1;
    return 0;
}

// ��������, �� � ���� ������ ��������� ������. ������� 1, ���� �
int checkNumber(const char * num)
{
    int hasHex = 0, hasH = 0;
    for(const char * s = num; *s; ++s)
    {
        char c = (char)tolower(*s);
        if (c >= 'a' && c <= 'f') hasHex = 1;  // �� � � �� xdigit
        if (c == 'g' || c > 'h') return 1;     // ��������� �������
        if (c == 'h')                           // h ������� ���� �������, ���� �
        {
            if (*(s+1)) return 1;
            hasH = 1;
        }
    }
    if (hasHex && !hasH) return 1;               // ���� � xdigit � ���� h
    return 0;
}

// ����� ����� (�������� �� ��������� ���������)
void parseLine(char* line)
{
    char tmp[1024];                  // ���������� �����
    for(char * s = line; *s; ++s)    // ���������� �����������
    {
        char c = *s;                 // �������� ������

        if (c == ' ' || c == '\t') { out(c,GRAY); continue; }   // ���� �� ��������� � ������ ��������
        else if (isdigit(c))                                    // ���� ���������� � ����� � �� �� ���� �����
        {
            char * t = tmp;                                     // �������� � tmp ��� ����� � ������
            while(isxdigit(*s) || isalpha(*s))
            {
                *t++ = *s++;
            }
            *t = 0;

            int error = checkNumber(tmp);                       // ���������� ����������
            if (!error) add(Numbers,tmp);                       // ���� �������� � �������� � �������
            outs(tmp, error ? RED : WHITE);                     // �������� �������� ��� �������
            if (*s) {--s; continue; } else break;               // ����������� �� ������� ���������� �������
        }

        else if (c == '\'' || c == '\"')                        // �� �� ���� �����
        {
            char quote = c, * t = tmp;                          // ������ ������ (��� ��� �����)
            for(*t++ = *s++;;)
            {
                char a = (*t++ = *s++);
                if (a == quote || a == 0)                       // ���� ������� ��� ����� �� ���� �����...
                {
                    *t = 0; 
                    if (a) add(Strings,tmp);                    // ���� �� ��������� ����� � ��������
                    outs(tmp, a ? YELLOW : RED); --s; break;    // ���� ����������� � �������� ��������
                }
            }
        }
        else if (c == ';')                                      // �������� � �� ��� �� ; �� ���� �����
        {
            add(Comments,s);                                    // ������� � �������
            outs(s,DARKBLUE);
            break;                                              // �� � �� ���� �����!
        }
        else if (c == '.')                                      // ���������, �� ���������� � ������
        {
            char * t = tmp, a = *s;
            for(*t++ = *s++; isdigit(*s) || isalpha(*s); ) 
                a = (*t++ = *s++);                              // �� ���� ��������������
            *t = 0; add(Directives,tmp); outs(tmp,MAGENTA);     // ���������, ������� �� �������, ��������
            if (a == 0) break; else --s;                        // ���� �� ����� ����� � �������� � �����
        }
        else if (isalpha(c) || strchr("?_@$%",c))               // �������������
        {
            char * t = tmp;
            for (*t++ = *s++; *s && (isdigit(*s) ||
                isalpha(*s) || strchr("?_@$%", *s)); *t++ = *s++);  // �� ���� ��������������
            *t = 0;

            // ������, �� �� � ���������, �������������, ������ ����, �������� � �������, �������� ��������
            if      (findIn(tmp,directives,  SIZE(directives)))   { add(Directives,tmp);   outs(tmp,MAGENTA); } 
            else if (findIn(tmp,instructions,SIZE(instructions))) { add(Instructions,tmp); outs(tmp,GREEN);   } 
            else if (findIn(tmp,registers,   SIZE(registers)))    { add(Registers,tmp);    outs(tmp,YELLOW);  } 
            else if (findIn(tmp,reserved,    SIZE(reserved)))     { add(Reserved,tmp);     outs(tmp,CYAN);    } 
            else                                                  { add(Identifiers,tmp);  outs(tmp,BLUE);    }
            if (*s == 0) break; else --s;            // ���� ����� ����� � �������� � �����
        } else if (strchr("+-*/",c))                 // ���������
        {
            tmp[0] = c;     tmp[1] = 0;     add(Operators,tmp);     outs(tmp,DARKRED);     // ������ �� �����
        } else if (strchr("[](){},:",c))             // ����������
        {
            tmp[0] = c;     tmp[1] = 0;     add(Delimeters,tmp);    outs(tmp,DARKMAGENTA); 
        } else {                                     // ���� ������� � �������!
            tmp[0] = c;     tmp[1] = 0;     add(Unknowns,tmp);      outs(tmp,RED);
        }
    }
    out('\n',GRAY);   // ʳ���� �����
}


int main(int argc, const char * argv[])
{
    SetConsoleOutputCP(1251);
    // �������� �������
    Numbers      = makeVec();    Strings      = makeVec();    Comments     = makeVec();
    Directives   = makeVec();    Instructions = makeVec();    Registers    = makeVec();
    Reserved     = makeVec();    Identifiers  = makeVec();    Operators    = makeVec();
    Delimeters   = makeVec();    Unknowns     = makeVec();

    // �������� ��'� ����� �� ��������� ����
    char fileName[_MAX_PATH+1]; // Filename
    // Get filename
    if (argc > 1)
        strncpy(fileName,argv[1],_MAX_PATH);
    else
    {
        printf("Enter the file name: ");
        fgets(fileName,_MAX_PATH+1,stdin);
        if (fileName[strlen(fileName)-1] == '\n')
            fileName[strlen(fileName)-1] = '\0';
    }

    FILE * in = fopen(fileName,"rt");
    if (in == NULL)
    {
        fprintf(stderr,"Can not open file `%s`\n",fileName);
        return 1;
    }

    // ������ �� ���������� �� �����
    char line[1024];
    int linesCount = 0;
    while(fgets(line,1024,in))
    {
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        if (line[0] == 0) continue;
        parseLine(line);
        if (++linesCount % 20 == 0) getchar();
    }
    fclose(in);

    // ������ ��� ������ �������
#define  OUTV(x) printf("%s:\n",#x); outv(x); puts("\n"); getchar();
    // �������� �� �������
    printf("----------------\n");
    OUTV(Numbers);
    OUTV(Strings);
    OUTV(Comments);
    OUTV(Directives);
    OUTV(Instructions);
    OUTV(Registers);
    OUTV(Reserved);
    OUTV(Identifiers);
    OUTV(Operators);
    OUTV(Delimeters);
    OUTV(Unknowns);

}

