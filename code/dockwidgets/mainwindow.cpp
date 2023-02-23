/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
#include <QtWidgets>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QtPrintSupport>
#endif
#endif

#include "mainwindow.h"
#include <QStringList>
#include <QSyntaxHighlighter>
//! [0]

//+++++++++++++++++++++++++++++++++begin 编译器相关定义+++++++++++++++++++++++++++++++++++++++++++++

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0
#define norw 18       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 11         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */
#define brnum_max 50  /* break语句最大数量 */

 /* 符号 */
enum symbol {
    nul, ident, number, plus, minus, dplus, dminus, xorsym,
    times, slash, percent, oddsym, eql, neq,
    lss, leq, gtr, geq, lparen,
    rparen, comma, semicolon, becomes,
    beginsym, endsym, ifsym, elsesym,
    whilesym, forsym, dosym, repeatsym, untilsym,
    continuesym, exitsym, breaksym,
    writesym, readsym, callsym, constsym,
    varsym, procsym,
};
#define symnum 41

/* 符号表中的类型 */
enum object {
    constant,
    variable,
    //procedure,
};

/* 虚拟机代码指令 */ //!!!可以去掉cal
enum fct {
    lit, opr, lod,
    sto, cal, ini,
    jmp, jpc,
};
#define fctnum 8

/* 虚拟机代码结构 */
struct instruction
{
    enum fct f; /* 虚拟机代码指令 */
    int l;      /* 引用层与声明层的层次差 */
    int a;      /* 根据f的不同而不同 */
};

char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, linenum;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */
int flagdepth;   /* 记录"{}"嵌套层数 */
bool flagcon, flagfor, flagtop;
int cx_break[brnum_max];
int brnum; /* 表示当前break语句个数 */
enum symbol tsym;

/* 符号表结构 */
struct tablestruct //!!!size也用不到，level都是0因为只有主函数
{
    char name[al];	    /* 名字 */
    enum object kind;	/* 类型：const，var或procedure */
    int val;            /* 数值，仅const使用 */
    int level;          /* 所处层，仅const不使用 */
    int adr;            /* 地址，仅const不使用 */
    int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

struct tablestruct table[txmax]; /* 符号表 */

int err;        /* 错误计数器 */

void compile_main();
void error(int n);
void getsym();
void getch();
void init();
void gen(enum fct x, int y, int z);
void test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
void block(int lev, int tx, bool* fsys);
void interpret();
void factor(bool* fsys, int* ptx, int lev);
void term(bool* fsys, int* ptx, int lev);
void condition(bool* fsys, int* ptx, int lev);
void expression(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev);
void listall();
void vardeclaration(int* ptx, int lev, int* pdx);
void constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);

//+++++++++++++++++++++++++++++++++end 编译器相关定义+++++++++++++++++++++++++++++++++++++++++++++

QString codestring; /* 存放main window中代码 */
QStringList codestring_list;
QStringList oprstring_list;
QString tempstring; /* 临时字符串变量 */
QString resultstring;     /* 存放result信息 */
QString errorstring;      /* 存放error信息 */
QString operationstring;  /* 存放operation信息 */
QString datastring;       /* 存放data信息 */
QString stackstring;       /* 存放stack信息 */
QString linestring;
QString oprstring = "";
char* linecharlist;
QByteArray qba;
QString dataname[txmax];
int datapoint,inpoint,oprpoint=0;
bool oprflag = false;
int b; /* 指令基址 */
int t; /* 栈顶指针 */
struct instruction i;	/* 存放当前指令 */
int s[stacksize];	/* 栈 */

//+++++++++++++++++++++++++++++++++begin 编译器相关函数+++++++++++++++++++++++++++++++++++++++++++++

/* 主程序开始 */
void compile_main()
{
    bool nxtlev[symnum];

    init();		/* 初始化 */
    err = 0;
    cc = linenum = cx = 0;
    ch = ' ';
    datapoint = inpoint = 0;

    //getsym();

    addset(nxtlev, declbegsys, statbegsys, symnum);
    nxtlev[endsym] = true;
    block(0, 0, nxtlev);	/* 处理分程序 */

    if (err == 0)
    {
        errorstring = "\n===Parsing success!===\n";

        listall();	 /* 输出所有代码 */
        interpret();	/* 调用解释执行程序 */
    }
    else
    {
        errorstring = QString::number(err) + " errors in pl/0 program!\n" + errorstring;
    }

}

/* 初始化 */
void init()
{
    int i;

    /* 设置单字符符号 */
    for (i = 0; i <= 255; i++)
    {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['%'] = percent;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym[','] = comma;
    ssym[';'] = semicolon;
    ssym['{'] = beginsym;
    ssym['}'] = endsym;

    /* 设置保留字名字,按照字母顺序，便于二分查找 */
    strcpy(&(word[0][0]), "break");
    strcpy(&(word[1][0]), "call");
    strcpy(&(word[2][0]), "const");
    strcpy(&(word[3][0]), "continue");
    strcpy(&(word[4][0]), "do");
    strcpy(&(word[5][0]), "else");
    strcpy(&(word[6][0]), "exit");
    strcpy(&(word[7][0]), "for");
    strcpy(&(word[8][0]), "if");
    strcpy(&(word[9][0]), "int");
    strcpy(&(word[10][0]), "odd");
    strcpy(&(word[11][0]), "read");
    strcpy(&(word[12][0]), "repeat");
    strcpy(&(word[13][0]), "until");
    strcpy(&(word[14][0]), "while");
    strcpy(&(word[15][0]), "write");
    strcpy(&(word[16][0]), "xor");

    /* 设置保留字符号 */
    wsym[0] = breaksym;
    wsym[1] = callsym;
    wsym[2] = constsym;
    wsym[3] = continuesym;
    wsym[4] = dosym;
    wsym[5] = elsesym;
    wsym[6] = exitsym;
    wsym[7] = forsym;
    wsym[8] = ifsym;
    wsym[9] = varsym;
    wsym[10] = oddsym;
    wsym[11] = readsym;
    wsym[12] = repeatsym;
    wsym[13] = untilsym;
    wsym[14] = whilesym;
    wsym[15] = writesym;
    wsym[16] = xorsym;

    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[ini][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");

    /* 设置符号集 */
    for (i = 0; i < symnum; i++)
    {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[varsym] = true;
    //declbegsys[procsym] = true;

    /* 设置语句开始符号集 */
    statbegsys[beginsym] = true;
    statbegsys[callsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[forsym] = true;
    statbegsys[repeatsym] = true;
    statbegsys[dosym] = true;
    statbegsys[ident] = true;
    statbegsys[readsym] = true;
    statbegsys[writesym] = true;
    statbegsys[breaksym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;
}

/* break存储初始化 */
void breakinit()
{
    int i;
    brnum = 0;
    for (i = 0; i < brnum_max; i++)
    {
        cx_break[i] = 0;
    }
}

/* 用数组实现集合的集合运算 */
int inset(int e, bool* s)
{
    return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        sr[i] = s1[i] || s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        sr[i] = s1[i] && (!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        sr[i] = s1[i] && s2[i];
    }
    return 0;
}

/* 出错处理，打印出错位置和错误编码 */
void error(int n)
{
    char space[81];
    memset(space, 32, 81);

    space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */

    errorstring += "++++++++here has error: " + QString::number(n) + "++++++++\n";

    err = err + 1;
}

/* 读取字符 */
/*
 * 过滤空格，读取一个字符
 * 被函数getsym调用
 */
void getch()
{
    if (cc == linestring.length() || linenum == 0) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
    {
        cc = 0;
        errorstring += QString::number(cx) + "  ";
        //ch = ' ';
        if (linenum < codestring_list.size())
        {
            linestring = codestring_list[linenum] + "\n";
            qba = linestring.toLocal8Bit();
            linecharlist = qba.data();
            linenum++;
            errorstring += linestring;
        }
    }
    ch = linecharlist[cc];
    cc++;
}

/* 词法分析，获取一个符号 */
void getsym()
{
    int i, j, k;
    bool flag; /* 注释符判断标记 */
    char ct;

    while (ch == ' ' || ch == 10 || ch == 9 || ch == '/')	/* 过滤空格、换行、制表和注释符 */
    {
        if (ch == '/') /* 可能是注释符 */
        {
            getch();
            if (ch == '*') // 已出现 '/*'
            {
                getch();
                flag = false; // 标记置false，未出现 ‘*’
                while (!flag || ch != '/')
                {
                    if (flag && ch != '/') flag = false; // 只是出现单个‘*’，标记置false
                    if (ch == '*') flag = true;
                    getch();
                }
                getch(); // 已经跳过注释
            }
            else /* 只是一个‘/’，非注释 */
            {
                sym = slash;
                return;
            }
        }
        else getch();
    }

    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
    {
        k = 0;
        do {
            if (k < al)
            {
                a[k] = ch;
                k++;              
            }
            getch();
        } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
        a[k] = 0;
        strcpy(id, a);
        i = 0;
        j = norw - 1;
        do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
            k = (i + j) / 2;
            if (strcmp(id, word[k]) <= 0)
            {
                j = k - 1;
            }
            if (strcmp(id, word[k]) >= 0)
            {
                i = k + 1;
            }
        } while (i <= j);
        if (i - 1 > j) /* 当前的单词是保留字 */
        {
            sym = wsym[k];
        }
        else /* 当前的单词是标识符 */
        {
            sym = ident;
        }
    }
    else
    {
        if (ch >= '0' && ch <= '9') /* 当前的单词是数字 */
        {
            k = 0;
            num = 0;
            sym = number;
            do {
                num = 10 * num + ch - '0';
                k++;
                getch();
            } while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
            k--;
            if (k > nmax) /* 数字位数太多 */
            {
                error(30);
            }
        }
        else
        {
            if (ch == '<')		/* 检测小于或小于等于符号 */
            {
                getch();
                if (ch == '=')
                {
                    sym = leq;
                    getch();
                }
                else
                {
                    sym = lss; /* 小于号 */
                }
            }
            else
            {
                if (ch == '>')		/* 检测大于或大于等于符号 */
                {
                    getch();
                    if (ch == '=')
                    {
                        sym = geq;
                        getch();
                    }
                    else
                    {
                        sym = gtr; /* 大于号 */
                    }
                }
                else
                {
                    if (ch == '!') /* 检测不等于号 */
                    {
                        getch();
                        if (ch == '=')
                        {
                            sym = neq;
                            getch();
                        }
                        else
                        {
                            sym = nul;	/* 不能识别的符号 */
                        }
                    }
                    else
                    {
                        if (ch == '=')		/* 检测等于号或赋值符号 */
                        {
                            getch();
                            if (ch == '=')
                            {
                                sym = eql;
                                getch();
                            }
                            else
                            {
                                sym = becomes;	/* 等号 */
                            }
                        }
                        else
                        {
                            if (ch == '+' || ch == '-')
                            {
                                ct = ch;
                                getch();
                                if (ch == ct)
                                {
                                    if (ch == '+') sym = dplus;
                                    if (ch == '-') sym = dminus;
                                }
                                else
                                {
                                    cc--;
                                    sym = ssym[ct];
                                }
                                getch();
                            }
                            else
                            {
                                sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
                                if (sym != endsym) getch();
                                else if (flagdepth > 0) getch();                            
                            }
                        }
                    }
                }
            }
        }
    }
}

/* 生成虚拟机代码 */
/*
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 */
void gen(enum fct x, int y, int z)
{
    if (cx >= cxmax)
    {
        operationstring = "Program is too long!\n"; /* 生成的虚拟机代码程序过长 */
        //exit(1);
    }
    if (z >= amax)
    {
        operationstring = "Displacement address is too big!\n"; /* 地址偏移越界 */
        //exit(1);
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++; //!!!cx指的是空位！
}

/* 测试当前符号是否合法 */
/*
 * 在语法分析程序的入口和出口处调用测试函数test，
 * 检查当前单词进入和退出该语法单位的合法性
 *
 * s1:	需要的单词集合
 * s2:	如果不是需要的单词，在某一出错状态时，
 *      可恢复语法分析继续正常工作的补充单词符号集合
 * n:  	错误号
 */
void test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        while ((!inset(sym, s1)) && (!inset(sym, s2)))
        {
            getsym();
        }
    }
}

/* 编译程序主体 */
/*
 * lev:    当前分程序所在层
 * tx:     符号表当前尾指针
 * fsys:   当前模块后继符号集合——FOLLOW集
 */
void block(int lev, int tx, bool* fsys)
{
    int i;
    int dx;                 /* 记录数据分配的相对地址 */
    int tx0;                /* 保留初始tx */
    int cx0;                /* 保留初始cx */
    bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
                               传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空间
                               传递给下级函数*/

    dx = 3;                 /* 三个空间用于存放静态链SL、动态链DL和返回地址RA  */
    tx0 = tx;		        /* 记录本层标识符的初始位置 */
    table[tx].adr = cx;	    /* 记录当前层代码的开始位置 */
    gen(jmp, 0, 0);         /* 产生跳转指令，跳转位置未知暂时填0 */

    if (lev > levmax)		/* 嵌套层数过多 */
    {
        error(33);
    }

    getsym();

    if (sym != beginsym)
    {
        error(32);
    }

    getsym();

    flagdepth = -1;
    flagcon = false;
    flagfor = false;

    if (sym == constsym || sym == varsym)
    {
        do {

            if (sym == constsym)	/* 遇到常量声明符号，开始处理常量声明 */
            {
                getsym();

                do {
                    constdeclaration(&tx, lev, &dx);	/* dx的值会被constdeclaration改变，使用指针 */
                    while (sym == comma)  /* 遇到逗号继续定义常量 */
                    {
                        getsym();
                        constdeclaration(&tx, lev, &dx);
                    }
                    if (sym == semicolon) /* 遇到分号结束定义常量 */
                    {
                        getsym();
                    }
                    else
                    {
                        error(5);   /* 漏掉了分号 */
                    }
                } while (sym  == ident);
            }

            if (sym == varsym)		/* 遇到变量声明符号，开始处理变量声明 */
            {
                getsym();

                do {
                    vardeclaration(&tx, lev, &dx);
                    while (sym == comma)
                    {
                        getsym();
                        vardeclaration(&tx, lev, &dx);
                    }
                    /*if (sym == semicolon)
                    {
                        getsym();
                    }
                    else*/
                    if (sym != semicolon)
                    {
                        error(5); /* 漏掉了分号 */
                    }
                } while (sym == ident);
            }

            //while (sym == procsym) /* 遇到过程声明符号，开始处理过程声明 */
            //{
            //	getsym();
            //	if (sym == ident)
            //	{
            //		enter(procedure, &tx, lev, &dx);	/* 填写符号表 */
            //		getsym();
            //	}
            //	else
            //	{
            //		error(4);	/* procedure后应为标识符 */
            //	}
            //	if (sym == semicolon)
            //	{
            //		getsym();
            //	}
            //	else
            //	{
            //		error(5);	/* 漏掉了分号 */
            //	}
            //	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
            //	nxtlev[semicolon] = true;
            //	block(lev + 1, tx, nxtlev); /* 递归调用 */
            //	if (sym == semicolon)
            //	{
            //		getsym();
            //		memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
            //		nxtlev[ident] = true;
            //		nxtlev[procsym] = true;
            //		test(nxtlev, fsys, 6);
            //	}
            //	else
            //	{
            //		error(5);	/* 漏掉了分号 */
            //	}
            //}
            memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
            nxtlev[ident] = true;
            nxtlev[semicolon] = true;
            nxtlev[endsym] = true;
            test(nxtlev, declbegsys, 7);
        } while (inset(sym, declbegsys));	/* 直到没有声明符号 */
    }

    //getsym();

    code[table[tx0].adr].a = cx;	/* 把前面生成的跳转语句的跳转位置改成当前位置 */ //!!!
    table[tx0].adr = cx;	        /* 记录当前过程代码地址 */
    table[tx0].size = dx;	        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    cx0 = cx;
    gen(ini, 0, dx);	            /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */

    for (i = 1; i <= tx; i++)
    {
        switch (table[i].kind)
        {
            case constant:
                datastring += "    " + QString::number(i) + " const " + table[i].name;
                datastring += " val = " + QString::number(table[i].val) + "\n";
                break;
            case variable:
                datastring += "    " + QString::number(i) + " int   " + table[i].name;
                datastring += " lev = " + QString::number(table[i].level)
                        + " addr = " + QString::number(table[i].adr) + "\n";
                break;
            //case procedure:
            //	printf("    %d proc  %s ", i, table[i].name);
            //	printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
            //	fprintf(ftable, "    %d proc  %s ", i, table[i].name);
            //	fprintf(ftable, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
            //	break;
        }
    }
    datastring += "\n";

    /* 语句后继符号为end */
    memcpy(nxtlev, fsys, sizeof(bool) * symnum);	/* 每个后继符号集合都包含上层后继符号集合，以便补救 */
    nxtlev[endsym] = true;
    nxtlev[elsesym] = true;
    nxtlev[semicolon] = true;
    nxtlev[rparen] = true;
    nxtlev[untilsym] = true;
    nxtlev[whilesym] = true;
    nxtlev[breaksym] = true;

    sym = beginsym;

    breakinit();

    statement(nxtlev, &tx, lev);
    gen(opr, 0, 0);	                    /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool) * symnum);	/* 分程序没有补救集合 */
    test(fsys, nxtlev, 8);            	/* 检测后继符号正确性 */
}

/* 在符号表中加入一项 */
/*
 * k:      标识符的种类为const，var或procedure
 * ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
 * lev:    标识符所在的层次
 * pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
 *
 */
void enter(enum object k, int* ptx, int lev, int* pdx)
{
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
    table[(*ptx)].kind = k;
    switch (k)
    {
        case constant:	/* 常量 */
            if (num > amax)
            {
                error(31);	/* 常数越界 */
                num = 0;
            }
            table[(*ptx)].val = num; /* 登记常数的值 */
            break;
        case variable:	/* 变量 */
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
    //case procedure:	/* 过程 */
    //	table[(*ptx)].level = lev;
    //	break;
    }
}

/* 查找标识符在符号表中的位置，从tx开始倒序查找标识符 */
/* 找到则返回在符号表中的位置，否则返回0
 *
 * id:    要查找的名字
 * tx:    当前符号表尾指针
 */
int position(char* id, int tx)
{
    int i;
    strcpy(table[0].name, id);
    i = tx;
    while (strcmp(table[i].name, id) != 0)
    {
        i--;
    }
    return i;
}

/* 常量声明处理 */
void constdeclaration(int* ptx, int lev, int* pdx)
{
    if (sym == ident)
    {
        getsym();
        if (sym == becomes)
        {
            getsym();
            if (sym == number)
            {
                enter(constant, ptx, lev, pdx);
                getsym();
            }
            else
            {
                error(2);	/* 常量声明中的=后应是数字 */
            }
        }
        else
        {
            error(3);	/* 常量声明中的标识符后应是= */
        }
    }
    else
    {
        error(4);	/* const后应是标识符 */
    }
}

/* 变量声明处理 */
void vardeclaration(int* ptx, int lev, int* pdx)
{
    if (sym == ident)
    {
        enter(variable, ptx, lev, pdx);	// 填写符号表
        getsym();
    }
    else
    {
        error(4);	/* var后面应是标识符 */
    }
}

/* 输出所有目标代码 */
void listall()
{
    int i;

    for (i = 0; i < cx; i++)
    {
        operationstring +=
           QString::number(i) + " "
                + mnemonic[code[i].f] + " "
                + QString::number(code[i].l) + " "
                + QString::number(code[i].a) + "\n";
    }
}

/* break语句地址回填 */
void backbreak(int endaddr)
{
    int i;
    for (i = 0; i < brnum; i++)
    {
        code[cx_break[i]].a = endaddr;
    }
}

/* 语句处理 */
void statement(bool* fsys, int* ptx, int lev)
{
    int i, cx1, cx2;
    bool nxtlev[symnum];

    if (sym == ident)	/* 准备按照赋值语句处理 */
    {
        i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
        if (i == 0)
        {
            error(11);	/* 标识符未声明 */
        }
        else
        {
            if (table[i].kind != variable)
            {
                error(12);	/* 赋值语句中，赋值号左部标识符应该是变量 */
                i = 0;
            }
            else
            {
                getsym();
                if (sym == dplus || sym == dminus)
                {
                    gen(lod, lev - table[i].level, table[i].adr);
                    gen(lit, 0, 1);
                    if (sym == dplus) gen(opr, 0, 2);
                    else gen(opr, 0, 3);
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    getsym();
                }
                else
                {
                    if (sym == becomes)
                    {
                        getsym();
                    }
                    else
                    {
                        error(3);	/* 没有检测到赋值符号 */
                    }
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    expression(nxtlev, ptx, lev);	/* 处理赋值符号右侧表达式 */
                }
                if (i != 0)
                {
                    /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
                    gen(sto, lev - table[i].level, table[i].adr);
                }
            }
        }
        //getsym();
        if (!flagfor)
        {
            if (sym != semicolon)
            {
                error(5);
            }
            else
            {
                getsym();
            }
        }
    }
    else
    {
        if (sym == readsym)	/* 准备按照read语句处理 */
        {
            getsym();
            if (sym != lparen)
            {
                error(23);	/* 格式错误，应是左括号 */
            }
            else
            {
                do {
                    getsym();
                    if (sym == ident)
                    {
                        i = position(id, *ptx);	/* 查找要读的变量 */
                    }
                    else
                    {
                        i = 0;
                    }
                    if (i == 0)
                    {
                        error(35);	/* read语句括号中的标识符应该是声明过的变量 */
                    }
                    else
                    {
                        dataname[datapoint] = table[i].name;
                        datapoint++;
                        gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
                        gen(sto, lev - table[i].level, table[i].adr);	/* 将栈顶内容送入变量单元中 */
                    }
                    getsym();
                } while (sym == comma);	/* 一条read语句可读多个变量 */
            }
            if (sym != rparen)
            {
                error(22);	/* 格式错误，应是右括号 */
                while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                {
                    getsym();
                }
            }
            else
            {
                getsym();
            }
            //getsym();
            if (sym != semicolon)
            {
                error(5);
            }
            else
            {
                getsym();
            }
        }
        else
        {
            if (sym == writesym)	/* 准备按照write语句处理 */
            {
                getsym();
                if (sym == lparen)
                {
                    do {
                        getsym();
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        nxtlev[rparen] = true;
                        nxtlev[comma] = true;
                        expression(nxtlev, ptx, lev);	/* 调用表达式处理 */
                        gen(opr, 0, 14);	/* 生成输出指令，输出栈顶的值 */
                        gen(opr, 0, 15);	/* 生成换行指令 */
                    } while (sym == comma);  /* 一条write可输出多个变量的值 */
                    if (sym != rparen)
                    {
                        error(22);	/* 格式错误，应是右括号 */
                    }
                    else
                    {
                        getsym();
                    }
                    //getsym();
                    if (sym != semicolon)
                    {
                        error(5);
                    }
                    else
                    {
                        getsym();
                    }
                }
            }
            else
            {
                //if (sym == callsym)	/* 准备按照call语句处理 */
                //{
                //	getsym();
                //	if (sym != ident)
                //	{
                //		error(14);	/* call后应为标识符 */
                //	}
                //	else
                //	{
                //		i = position(id, *ptx);
                //		if (i == 0)
                //		{
                //			error(11);	/* 过程名未找到 */
                //		}
                //		else
                //		{
                //			if (table[i].kind == procedure)
                //			{
                //				gen(cal, lev - table[i].level, table[i].adr);	/* 生成call指令 */
                //			}
                //			else
                //			{
                //				error(15);	/* call后标识符类型应为过程 */
                //			}
                //		}
                //		getsym();
                //	}
                //	//getsym();
                //	/*if (sym != semicolon)
                //	{
                //		error(5);
                //	}
                //	else
                //	{
                //		getsym();
                //	}*/
                //}
                //else
                //{
                    if (sym == ifsym)	/* 准备按照if语句处理 */
                    {
                        getsym();
                        if (sym != lparen)
                        {
                            error(23); /* 格式错误，应是左括号 */
                        }
                        else
                        {
                            getsym();
                            memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                            //nxtlev[thensym] = true;
                            //nxtlev[dosym] = true;	/* 后继符号为then或do */
                            nxtlev[rparen] = true;
                            nxtlev[elsesym] = true;
                            nxtlev[endsym] = true;
                            condition(nxtlev, ptx, lev); /* 调用条件处理 */
                        }
                        if (sym != rparen)
                        {
                            error(22);  /* 格式错误，应是右括号 */
                            while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                            {
                                getsym();
                            }
                        }
                        else
                        {
                            getsym();
                            cx1 = cx;	/* 保存当前指令地址 */
                            gen(jpc, 0, 0);	/* 生成条件跳转指令，跳转地址未知，暂时写0 */
                            statement(fsys, ptx, lev);	/* 处理if后的语句 */
                            if (sym == elsesym)
                            {
                                getsym();
                                cx2 = cx;
                                gen(jmp, 0, 0);  /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                                code[cx1].a = cx;
                                statement(fsys, ptx, lev);	/* 处理else后的语句 */
                                code[cx2].a = cx;  /* 经statement处理后，cx为else后语句执行完的位置，它正是前面未定的跳转地址，此时进行回填 */
                            }
                            else
                            {
                                code[cx1].a = cx;
                            }
                        }
                    }
                    else
                    {
                        if (sym == beginsym)	/* 准备按照复合语句处理 */
                        {
                            flagdepth++;
                            getsym();
                            memcpy(nxtlev, fsys, sizeof(bool) * symnum);

                            nxtlev[semicolon] = true;
                            nxtlev[endsym] = true;	/* 后继符号为分号或end */

                            statement(nxtlev, ptx, lev);  /* 对begin与end之间的语句进行分析处理 */
                            /* 如果分析完一句后遇到语句开始符或分号，则循环分析下一句语句 */
                            while (inset(sym, statbegsys))
                            {
                                statement(nxtlev, ptx, lev);
                            }
                            if (sym == endsym)
                            {
                                flagdepth--;
                                getsym();
                            }
                            else
                            {
                                error(17);	/* 缺少end */
                            }
                        }
                        else
                        {
                            if (sym == whilesym)	/* 准备按照while语句处理 */
                            {
                                cx1 = cx;	/* 保存判断条件操作的位置 */
                                getsym();
                                if (sym != lparen)
                                {
                                    error(23); /* 格式错误，应是左括号 */
                                }
                                else
                                {
                                    getsym();
                                    memcpy(nxtlev, fsys, sizeof(bool)* symnum);                         
                                    nxtlev[rparen] = true;
                                    condition(nxtlev, ptx, lev);	/* 调用条件处理 */
                                    cx2 = cx;	/* 保存循环体的结束的下一个位置 */
                                    gen(jpc, 0, 0);	/* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
                                }
                                if (sym != rparen)
                                {
                                    error(22);  /* 格式错误，应是右括号 */
                                    while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                                    {
                                        getsym();
                                    }
                                }
                                else
                                {
                                    getsym();
                                    statement(fsys, ptx, lev);	/* 循环体 */
                                    gen(jmp, 0, cx1);	/* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
                                    code[cx2].a = cx;	/* 回填跳出循环的地址 */
                                    backbreak(cx);
                                    breakinit();
                                }
                            }
                            else
                            {
                                if (sym == forsym)	/* 准备按照for语句处理 */
                                {
                                    getsym();
                                    if (sym != lparen)
                                    {
                                        error(23); /* 格式错误，应是左括号 */
                                    }
                                    else
                                    {
                                        flagfor = false;
                                        getsym();
                                        memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                        nxtlev[rparen] = true;
                                        nxtlev[semicolon] = true;
                                        statement(fsys, ptx, lev);
                                        /*if (sym != semicolon)
                                        {
                                            error(5);
                                        }*/

                                        cx1 = cx;
                                        memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                        nxtlev[rparen] = true;
                                        nxtlev[semicolon] = true;
                                        condition(nxtlev, ptx, lev);	/* 调用条件处理 */
                                        if (sym != semicolon)
                                        {
                                            error(5);
                                        }

                                        cx2 = cx;
                                        gen(jpc, 0, 0);
                                        gen(jmp, 0, 0);
                                        getsym();
                                        flagfor = true;
                                        memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                        nxtlev[rparen] = true;
                                        nxtlev[beginsym] = true;
                                        nxtlev[endsym] = true;
                                        statement(fsys, ptx, lev);
                                        gen(jmp, 0, cx1);
                                        code[cx2+1].a = cx;
                                        flagfor = false;
                                    }
                                    if (sym != rparen)
                                    {
                                        error(22);  /* 格式错误，应是右括号 */
                                        while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                                        {
                                            getsym();
                                        }
                                    }
                                    else
                                    {
                                        getsym();
                                        nxtlev[semicolon] = true;
                                        nxtlev[endsym] = true;
                                        statement(fsys, ptx, lev);	/* 循环体 */
                                        gen(jmp, 0, cx2+2);	/* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
                                        code[cx2].a = cx;	/* 回填跳出循环的地址 */
                                        backbreak(cx);
                                        breakinit();
                                    }
                                }
                                else
                                {
                                    if (sym == repeatsym)
                                    {
                                        getsym();
                                        memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                        nxtlev[lparen] = true;
                                        nxtlev[rparen] = true;
                                        nxtlev[semicolon] = true;
                                        nxtlev[untilsym] = true;
                                        cx1 = cx;
                                        statement(fsys, ptx, lev);
                                        if (sym != untilsym)
                                        {
                                            error(66);
                                        }
                                        else
                                        {
                                            getsym();
                                            if (sym != lparen)
                                            {
                                                error(23); /* 格式错误，应是左括号 */
                                            }
                                            else
                                            {
                                                getsym();
                                                memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                                nxtlev[rparen] = true;
                                                nxtlev[semicolon] = true;
                                                condition(nxtlev, ptx, lev);	/* 调用条件处理 */
                                                gen(jpc, 0, cx1);
                                                backbreak(cx);
                                                breakinit();
                                            }
                                            if (sym != rparen)
                                            {
                                                error(22);  /* 格式错误，应是右括号 */
                                                while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                                                {
                                                    getsym();
                                                }
                                            }
                                            else
                                            {
                                                getsym();
                                                if (sym != semicolon)
                                                {
                                                    error(5);
                                                }
                                                else getsym();
                                            }
                                        }

                                    }
                                    else
                                    {
                                        if (sym == dosym)
                                        {
                                            getsym();
                                            memcpy(nxtlev, fsys, sizeof(bool)* symnum);
                                            nxtlev[lparen] = true;
                                            nxtlev[rparen] = true;
                                            nxtlev[semicolon] = true;
                                            nxtlev[untilsym] = true;
                                            cx1 = cx;
                                            statement(fsys, ptx, lev);
                                            if (sym != whilesym)
                                            {
                                                error(77);
                                            }
                                            else
                                            {
                                                getsym();
                                                if (sym != lparen)
                                                {
                                                    error(23); /* 格式错误，应是左括号 */
                                                }
                                                else
                                                {
                                                    getsym();
                                                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                                                    nxtlev[rparen] = true;
                                                    nxtlev[semicolon] = true;
                                                    condition(nxtlev, ptx, lev);	/* 调用条件处理 */
                                                    gen(jpc, 0, cx+2);
                                                    gen(jmp, 0, cx1);
                                                    backbreak(cx);
                                                    breakinit();
                                                }
                                                if (sym != rparen)
                                                {
                                                    error(22);  /* 格式错误，应是右括号 */
                                                    while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
                                                    {
                                                        getsym();
                                                    }
                                                }
                                                else
                                                {
                                                    getsym();
                                                    if (sym != semicolon)
                                                    {
                                                        error(5);
                                                    }
                                                    else getsym();
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if (sym == breaksym)
                                            {
                                                brnum++;
                                                cx_break[brnum-1] = cx;
                                                gen(jmp, 0, 0);
                                                getsym();
                                                if (sym != semicolon)
                                                {
                                                    error(5);
                                                }
                                                else getsym();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                //}
            }
        }
    }
    memset(nxtlev, 0, sizeof(bool) * symnum);	/* 语句结束无补救集合 */
    test(fsys, nxtlev, 19);	/* 检测语句结束的正确性 */
}

/* 表达式处理 */
void expression(bool* fsys, int* ptx, int lev)
{
    enum symbol addop;	/* 用于保存正负号 */
    bool nxtlev[symnum];

    if (sym == plus || sym == minus)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
    {
        addop = sym;	/* 保存开头的正负号 */
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        term(nxtlev, ptx, lev);	/* 处理项 */
        if (addop == minus)
        {
            gen(opr, 0, 1);	/* 如果开头为负号生成取负指令 */
        }
    }
    else	/* 此时表达式被看作项的加减 */
    {
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        nxtlev[xorsym] = true;
        term(nxtlev, ptx, lev);	/* 处理项 */
    }
    while (sym == plus || sym == minus || sym == xorsym)
    {
        addop = sym;
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        nxtlev[xorsym] = true;
        term(nxtlev, ptx, lev);	/* 处理项 */
        if (addop == plus)
        {
            gen(opr, 0, 2);	/* 生成加法指令 */
        }
        else if (addop == minus)
        {
            gen(opr, 0, 3);	/* 生成减法指令 */
        }
        else
        {
            gen(opr, 0, 17);	/* 生成异或指令 */
        }
    }
}

/* 项处理 */
void term(bool* fsys, int* ptx, int lev)
{
    enum symbol mulop;	/* 用于保存乘除法符号 */
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    nxtlev[percent] = true;
    factor(nxtlev, ptx, lev);	/* 处理因子 */
    while (sym == times || sym == slash || sym == percent)
    {
        mulop = sym;
        getsym();
        factor(nxtlev, ptx, lev);
        if (mulop == times)
        {
            gen(opr, 0, 4);	/* 生成乘法指令 */
        }
        else if(mulop == slash)
        {
            gen(opr, 0, 5);	/* 生成除法指令 */
        }
        else
        {
            gen(opr, 0, 6);
        }
    }
}

/* 因子处理 */
void factor(bool* fsys, int* ptx, int lev)
{
    int i;
    bool nxtlev[symnum];
    test(facbegsys, fsys, 24);	/* 检测因子的开始符号 */
    while (inset(sym, facbegsys)) 	/* 循环处理因子 */
    {
        if (sym == ident)	/* 因子为常量或变量 */
        {
            i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
            if (i == 0)
            {
                error(11);	/* 标识符未声明 */
            }
            else
            {
                switch (table[i].kind)
                {
                case constant:	/* 标识符为常量 */
                    gen(lit, 0, table[i].val);	/* 直接把常量的值入栈 */
                    break;
                case variable:	/* 标识符为变量 */
                    gen(lod, lev - table[i].level, table[i].adr);	/* 找到变量地址并将其值入栈 */
                    break;
                //case procedure:	/* 标识符为过程 */
                //	error(21);	/* 不能为过程 */
                //	break;
                }
            }
            getsym();
        }
        else
        {
            if (sym == number)	/* 因子为数 */
            {
                if (num > amax)
                {
                    error(301); /* 数越界 */
                    num = 0;
                }
                if (flagcon) num = 0;
                gen(lit, 0, num);
                if (!flagcon) getsym();
                else
                {
                    sym = rparen;
                    flagcon = false;
                    //break;
                }
            }
            else
            {
                if (sym == lparen)	/* 因子为表达式 */
                {
                    getsym();
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    expression(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsym();
                    }
                    else
                    {
                        error(22);	/* 缺少右括号 */
                    }
                }
            }
        }
        memset(nxtlev, 0, sizeof(bool) * symnum);
        nxtlev[lparen] = true;
        test(fsys, nxtlev, 24); /* 一个因子处理完毕，遇到的单词应在fsys集合中 */
                                /* 如果不是，报错并找到下一个因子的开始，使语法分析可以继续运行下去 */
    }
}

/* 条件处理 */
void condition(bool* fsys, int* ptx, int lev)
{
    enum symbol relop;
    bool nxtlev[symnum];

    if (sym == oddsym)	/* 准备按照odd运算处理 */
    {
        getsym();
        expression(fsys, ptx, lev);
        gen(opr, 0, 7);	/* 生成odd指令 */
    }
    else
    {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        expression(nxtlev, ptx, lev);
        if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq && sym != rparen)
        {
            error(20); /* 应该为关系运算符 */
        }
        else
        {
            if (sym == rparen)
            {
                flagcon = true;
                sym = gtr;
            }

            relop = sym;
            if (!flagcon) getsym();
            else sym = number;
            expression(fsys, ptx, lev);
            switch (relop)
            {
                case eql:
                    gen(opr, 0, 8);
                    break;
                case neq:
                    gen(opr, 0, 9);
                    break;
                case lss:
                    gen(opr, 0, 10);
                    break;
                case geq:
                    gen(opr, 0, 11);
                    break;
                case gtr:
                    gen(opr, 0, 12);
                    break;
                case leq:
                    gen(opr, 0, 13);
                    break;
            }
        }
    }
}

/* 输入对话框 */
int input()
{
    QInputDialog *inputw = nullptr;
    QString dlgTitle = "输入变量";
    QString txtLabel = "请输入变量" + dataname[inpoint] + "的值：";
    QString defaultInput = "";
    QLineEdit::EchoMode echoMode = QLineEdit::Normal; //正常文字输入
    QString datavals = inputw->getText(inputw, dlgTitle,txtLabel, echoMode,defaultInput);
    return datavals.toInt();
}

/* 解释程序 */
void interpret()
{
    int p = 0; /* 指令指针 */
    int b = 1; /* 指令基址 */
    int t = 0; /* 栈顶指针 */
    struct instruction i;	/* 存放当前指令 */
    int s[stacksize];	/* 栈 */

    stackstring = "";
    resultstring += "Start smallC\n";

    s[0] = 0; /* s[0]不用 */
    s[1] = 0; /* 主程序的三个联系单元均置为0 */
    s[2] = 0;
    s[3] = 0;
    do {
        i = code[p];	/* 读当前指令 */
        p = p + 1;
        switch (i.f)
        {
            case lit:	/* 将常量a的值取到栈顶 */
                t = t + 1;
                s[t] = i.a;
                break;
            case opr:	/* 数学、逻辑运算 */
                switch (i.a)
                {
                    case 0:  /* 函数调用结束后返回 */
                        t = b - 1;
                        p = s[t + 3];
                        b = s[t + 2];
                        break;
                    case 1: /* 栈顶元素取反 */
                        s[t] = -s[t];
                        break;
                    case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
                        t = t - 1;
                        s[t] = s[t] + s[t + 1];
                        break;
                    case 3:/* 次栈顶项减去栈顶项 */
                        t = t - 1;
                        s[t] = s[t] - s[t + 1];
                        break;
                    case 4:/* 次栈顶项乘以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] * s[t + 1];
                        break;
                    case 5:/* 次栈顶项除以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] / s[t + 1];
                        break;
                    case 6:/* 次栈顶项对栈顶项取余 */
                        t = t - 1;
                        s[t] = s[t] % s[t + 1];
                        break;
                    case 7:/* 栈顶元素的奇偶判断 */
                        s[t] = s[t] % 2;
                        break;
                    case 8:/* 次栈顶项与栈顶项是否相等 */
                        t = t - 1;
                        s[t] = (s[t] == s[t + 1]);
                        break;
                    case 9:/* 次栈顶项与栈顶项是否不等 */
                        t = t - 1;
                        s[t] = (s[t] != s[t + 1]);
                        break;
                    case 10:/* 次栈顶项是否小于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] < s[t + 1]);
                        break;
                    case 11:/* 次栈顶项是否大于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] >= s[t + 1]);
                        break;
                    case 12:/* 次栈顶项是否大于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] > s[t + 1]);
                        break;
                    case 13: /* 次栈顶项是否小于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] <= s[t + 1]);
                        break;
                    case 14:/* 栈顶值输出 */
                        resultstring += QString::number(s[t]);
                        t = t - 1;
                        break;
                    case 15:/* 输出换行符 */
                        resultstring += "\n";
                        break;
                    case 16:/* 读入一个输入置于栈顶 */
                        resultstring += dataname[inpoint] + ":";
                        t = t + 1;
                        s[t] = input();
                        inpoint++;
                        resultstring += QString::number(s[t]) + "\n";
                        break;
                    case 17:/* 次栈顶项与栈顶项做异或运算 */
                        t = t - 1;
                        s[t] = s[t] ^ s[t + 1];
                        break;
                }
            break;
            case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                t = t + 1;
                s[t] = s[base(i.l, s, b) + i.a];
                break;
            case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                s[base(i.l, s, b) + i.a] = s[t];
                t = t - 1;
                break;
            case cal:	/* 调用子过程 */
                s[t + 1] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
                s[t + 2] = b;	/* 将本过程基地址入栈，即建立动态链 */
                s[t + 3] = p;	/* 将当前指令指针入栈，即保存返回地址 */
                b = t + 1;	/* 改变基地址指针值为新过程的基地址 */
                p = i.a;	/* 跳转 */
                break;
            case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
                t = t + i.a;
                break;
            case jmp:	/* 直接跳转 */
                p = i.a;
                break;
            case jpc:	/* 条件跳转 */
                if (s[t] == 0)
                    p = i.a;
                t = t - 1;
                break;
        }
    } while (p != 0);

    for (int i=0;i<t;i++)
    {
        stackstring += QString::number(i) + " " + QString::number(s[i]) + "\n";
    }
    resultstring += "End smallC\n";
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)
{
    int b1;
    b1 = b;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}

//+++++++++++++++++++++++++++++++++end 编译器相关函数+++++++++++++++++++++++++++++++++++++++++++++

//! [1]
MainWindow::MainWindow(): textEdit(new QTextEdit)
{
    //QSyntaxHighlighter * h = new QSyntaxHighlighter(textEdit->document());//传一个QTextDocument

    setCentralWidget(textEdit);
    this->resize( QSize( 1800, 1200 ));
    QFont font = QFont("Consolas",12,6);
    textEdit->setFont(font);

    createActions();
    createStatusBar();
    createInfoWindows();

    setWindowTitle(tr("smallC_compiler"));

    setUnifiedTitleAndToolBarOnMac(true);
}
//! [1]

//! [2]
void MainWindow::newFile()
{
    textEdit->clear();
}
//! [2]

//! [3]
void MainWindow::openFile()
{
    //textEdit->clear();

    /*
    getOpenFileName函数说明
    函数原形： QStringList QFileDialog::getOpenFileNames(
    QWidget * parent = 0,
    const QString & caption = QString(),	//  打开文件对话框的标题
    const QString & dir = QString(),			//	查找目录
    const QString & filter = QString(),		//  设置需要过滤的文件格式
    QString * selectedFilter = 0,
    Options options = 0) [static]
    */
    //---获取文件名
    QString fileName = QFileDialog :: getOpenFileName(this, NULL, NULL, "*.txt");

    //---打开文件并读取文件内容
    QFile file(fileName);

    //--打开文件成功
    if (file.open(QIODevice ::ReadOnly | QIODevice ::Text))
    {
        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            //---QtextEdit按行显示文件内容
            textEdit->append(textStream.readLine());
        }
    }
    else	//---打开文件失败
    {
        /*
        information函数参数说明：
        函数原型
        QMessageBox::information(
        QWidget * parent,
        const QString & title,					//--- 标题
        const QString & text,					//---显示内容
        StandardButtons buttons = Ok,	//---OK按钮
        StandardButton defaultButton = NoButton)
        [static]
        */
        QMessageBox ::information(NULL, NULL, "open file error");
    }

}
//! [3]

//! [4]
void MainWindow::print()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QTextDocument *document = textEdit->document();
    QPrinter printer;

    QPrintDialog dlg(&printer, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    document->print(&printer);
    statusBar()->showMessage(tr("Ready"), 2000);
#endif
}
//! [4]

//! [5]
void MainWindow::run()
{
    codestring = textEdit->toPlainText();
    codestring_list = codestring.split("\n");
    tempstring = "";
    resultstring = "";
    errorstring = "";
    operationstring = "";
    datastring = "";

    compile_main();

    operationBlock->setPlainText(operationstring);
    errorBlock->setPlainText(errorstring);
    resultBlock->setPlainText(resultstring);
    dataBlock->setPlainText(datastring);
    stackBlock->setPlainText(stackstring);

    oprpoint = 0;
}
//! [5]

//! [6]
void MainWindow::steprun()
{
    //int p = 0; /* 指令指针 */
    stackBlock->clear();
    stackstring = "";
    operationBlock->clear();
    oprstring_list = operationstring.split("\n");

    if (!oprflag)
    {
        s[0] = 0; /* s[0]不用 */
        s[1] = 0; /* 主程序的三个联系单元均置为0 */
        s[2] = 0;
        s[3] = 0;
        b = 1;
        t = 0;
        oprpoint = 0;
        stackstring = "";
        oprflag = true;
    }

    //oprstring = "";

    do {
        i = code[oprpoint];	/* 读当前指令 */

        for (int li=0;li<oprstring_list.size();li++)
        {
            if (li == oprpoint)
                operationBlock->append("<font color=\"#25e5e1\">" + oprstring_list[li] + "</font>");
            else
                operationBlock->append(oprstring_list[li]);
        }

        oprpoint++;
        switch (i.f)
        {
            case lit:	/* 将常量a的值取到栈顶 */
                t = t + 1;
                s[t] = i.a;
                break;
            case opr:	/* 数学、逻辑运算 */
                switch (i.a)
                {
                    case 0:  /* 函数调用结束后返回 */
                        t = b - 1;
                        oprpoint = s[t + 3];
                        b = s[t + 2];
                        break;
                    case 1: /* 栈顶元素取反 */
                        s[t] = -s[t];
                        break;
                    case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
                        t = t - 1;
                        s[t] = s[t] + s[t + 1];
                        break;
                    case 3:/* 次栈顶项减去栈顶项 */
                        t = t - 1;
                        s[t] = s[t] - s[t + 1];
                        break;
                    case 4:/* 次栈顶项乘以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] * s[t + 1];
                        break;
                    case 5:/* 次栈顶项除以栈顶项 */
                        t = t - 1;
                        s[t] = s[t] / s[t + 1];
                        break;
                    case 6:/* 次栈顶项对栈顶项取余 */
                        t = t - 1;
                        s[t] = s[t] % s[t + 1];
                        break;
                    case 7:/* 栈顶元素的奇偶判断 */
                        s[t] = s[t] % 2;
                        break;
                    case 8:/* 次栈顶项与栈顶项是否相等 */
                        t = t - 1;
                        s[t] = (s[t] == s[t + 1]);
                        break;
                    case 9:/* 次栈顶项与栈顶项是否不等 */
                        t = t - 1;
                        s[t] = (s[t] != s[t + 1]);
                        break;
                    case 10:/* 次栈顶项是否小于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] < s[t + 1]);
                        break;
                    case 11:/* 次栈顶项是否大于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] >= s[t + 1]);
                        break;
                    case 12:/* 次栈顶项是否大于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] > s[t + 1]);
                        break;
                    case 13: /* 次栈顶项是否小于等于栈顶项 */
                        t = t - 1;
                        s[t] = (s[t] <= s[t + 1]);
                        break;
                    case 14:/* 栈顶值输出 */
                        resultstring += QString::number(s[t]);
                        t = t - 1;
                        break;
                    case 15:/* 输出换行符 */
                        resultstring += "\n";
                        break;
                    case 16:/* 读入一个输入置于栈顶 */
                        resultstring += dataname[inpoint] + ":";
                        t = t + 1;
                        s[t] = input();
                        inpoint++;
                        resultstring += QString::number(s[t]) + "\n";
                        break;
                    case 17:/* 次栈顶项与栈顶项做异或运算 */
                        t = t - 1;
                        s[t] = s[t] ^ s[t + 1];
                        break;
                }
            break;
            case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                t = t + 1;
                s[t] = s[base(i.l, s, b) + i.a];
                break;
            case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                s[base(i.l, s, b) + i.a] = s[t];
                t = t - 1;
                break;
            case cal:	/* 调用子过程 */
                s[t + 1] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
                s[t + 2] = b;	/* 将本过程基地址入栈，即建立动态链 */
                s[t + 3] = oprpoint;	/* 将当前指令指针入栈，即保存返回地址 */
                b = t + 1;	/* 改变基地址指针值为新过程的基地址 */
                oprpoint = i.a;	/* 跳转 */
                break;
            case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
                t = t + i.a;
                break;
            case jmp:	/* 直接跳转 */
                oprpoint = i.a;
                break;
            case jpc:	/* 条件跳转 */
                if (s[t] == 0)
                    oprpoint = i.a;
                t = t - 1;
                break;
        }
        if (oprpoint != 0)
        {
            //oprflag = true;
            break;
        }
    } while (oprpoint != 0);


    stackstring += QString::number(0) + " " + QString::number(s[0]) + "\n";
    stackstring += QString::number(1) + " " + QString::number(s[1]) + "\n";
    stackstring += QString::number(2) + " " + QString::number(s[2]) + "\n";
    stackstring += QString::number(3) + " " + QString::number(s[3]) + "\n";
    for(int i=4;i<t;i++)
    {
        stackstring += QString::number(i) + " " + QString::number(s[i]) + "\n";
    }

    stackBlock->setPlainText(stackstring);

    if (oprpoint == 0)
    {
        oprflag = false;
    }
    //oprpoint++;
}
//! [6]

//! [7]
void MainWindow::save()
{
    QMimeDatabase mimeDatabase;
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), ".txt",
                        mimeDatabase.mimeTypeForName("text/txt").filterString());
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("smallC_compiler"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream out(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    out << "\n+++++++++++++++++++++++++code+++++++++++++++++++++++++\n";
    out << textEdit -> toPlainText();
    out << "\n+++++++++++++++++++++++++result+++++++++++++++++++++++++\n";
    out << resultBlock -> toPlainText();
    out << "\n+++++++++++++++++++++++++error+++++++++++++++++++++++++\n";
    out << errorBlock -> toPlainText();
    out << "\n+++++++++++++++++++++++++operation+++++++++++++++++++++++++\n";
    out << operationBlock -> toPlainText();
    out << "\n+++++++++++++++++++++++++data+++++++++++++++++++++++++\n";
    out << dataBlock -> toPlainText();
    QGuiApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);

}
//! [7]

//! [8]
void MainWindow::undo()
{
    QTextDocument *document = textEdit->document();
    document->undo();
}
//! [8]

//! [9]
void MainWindow::about()
{
   QMessageBox::about(this, tr("About Dock Widgets"),
            tr("The <b>Dock Widgets</b> example demonstrates how to "
               "use Qt's dock widgets. You can enter your own text, "
               "click a customer to add a customer name and "
               "address, and click standard paragraphs to add them."));
}
//! [9]

//! [10]
void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("code-new", QIcon(":/images/new.png"));
    QAction *newFileAct = new QAction(newIcon, tr("&New File"), this);
    newFileAct->setShortcuts(QKeySequence::New);
    newFileAct->setStatusTip(tr("Create a new code file"));
    connect(newFileAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newFileAct);
    fileToolBar->addAction(newFileAct);

    const QIcon openFileIcon = QIcon::fromTheme("code-open", QIcon(":/images/openfile.png"));
    QAction *openFileAct = new QAction(openFileIcon, tr("&Open file"), this);
    openFileAct->setShortcuts(QKeySequence::Open);
    openFileAct->setStatusTip(tr("Open a code file from your desktop"));
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openFileAct);
    fileToolBar->addAction(openFileAct);

    const QIcon saveIcon = QIcon::fromTheme("code-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save..."), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current code in the main window"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon printIcon = QIcon::fromTheme("code-print", QIcon(":/images/print.png"));
    QAction *printAct = new QAction(printIcon, tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the current code in the main window"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);
    fileMenu->addAction(printAct);
    fileToolBar->addAction(printAct);

    fileMenu->addSeparator();

    const QIcon runIcon = QIcon::fromTheme("code-run", QIcon(":/images/run.png"));
    QAction *runAct = new QAction(runIcon, tr("&Run"), this);
    //runAct->setShortcuts(QKeySequence::Run);
    runAct->setStatusTip(tr("Run the code in main window"));
    connect(runAct, &QAction::triggered, this, &MainWindow::run);
    fileMenu->addAction(runAct);
    fileToolBar->addAction(runAct);

    const QIcon steprunIcon = QIcon::fromTheme("code-steprun", QIcon(":/images/steprun.png"));
    QAction *steprunAct = new QAction(steprunIcon, tr("&Step run"), this);
    //steprunAct->setShortcuts(QKeySequence::Steprun);
    steprunAct->setStatusTip(tr("Run the code in operation window step by step"));
    connect(steprunAct, &QAction::triggered, this, &MainWindow::steprun);
    fileMenu->addAction(steprunAct);
    fileToolBar->addAction(steprunAct);

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));
    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png"));
    QAction *undoAct = new QAction(undoIcon, tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    editMenu->addAction(undoAct);
    editToolBar->addAction(undoAct);

    viewMenu = menuBar()->addMenu(tr("&View"));

    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}
//! [10]

//! [11]
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}
//! [11]

//! [12]
void MainWindow::createInfoWindows()
{
    QDockWidget *textblock = new QDockWidget(tr("Operation"), this);
    textblock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    operationBlock = new QTextEdit();
    textblock->setWidget(operationBlock);
    addDockWidget(Qt::RightDockWidgetArea, textblock);
    viewMenu->addAction(textblock->toggleViewAction());
    QFont font1 = QFont("Consolas",10,6);
    operationBlock->setFont(font1);

    textblock = new QDockWidget(tr("Error"), this);
    textblock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    errorBlock = new QTextEdit();
    textblock->setWidget(errorBlock);
    addDockWidget(Qt::RightDockWidgetArea, textblock);
    viewMenu->addAction(textblock->toggleViewAction());
    QFont font2 = QFont("Consolas",12,6);
    errorBlock->setFont(font2);

    textblock = new QDockWidget(tr("Result"), this);
    textblock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resultBlock = new QTextEdit();
    textblock->setWidget(resultBlock);
    addDockWidget(Qt::RightDockWidgetArea, textblock);
    viewMenu->addAction(textblock->toggleViewAction());
    QFont font3 = QFont("Consolas",12,6);
    resultBlock->setFont(font3);

    textblock = new QDockWidget(tr("Data"), this);
    textblock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dataBlock = new QTextEdit();
    textblock->setWidget(dataBlock);
    addDockWidget(Qt::RightDockWidgetArea, textblock);
    viewMenu->addAction(textblock->toggleViewAction());
    QFont font4 = QFont("Consolas",12,6);
    dataBlock->setFont(font4);

    textblock = new QDockWidget(tr("Stack"), this);
    textblock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    stackBlock = new QTextEdit();
    textblock->setWidget(stackBlock);
    addDockWidget(Qt::RightDockWidgetArea, textblock);
    viewMenu->addAction(textblock->toggleViewAction());
    QFont font5 = QFont("Consolas",10,6);
    dataBlock->setFont(font5);
}
//! [12]
