//
//	VM(Virtual Machine Code) to i386 Native Instruction trans-table
//								(C) yaneurao(M.Isozaki) 2000/01/18-2000/01/21
//

#ifndef __VM2I386TABLE__
#define __VM2I386TABLE__

//////////////////////////////////////////////////////////////////////////////
//
//	 technologies of this rapid translation method
//				are developed by yaneurao(M.Isozaki)
//							ALL RIGHT RESERVED Y2K
//
//////////////////////////////////////////////////////////////////////////////

#define V_DWORD 0xf000			//  start line of dword command
#define V_THIS(x) (0xfd00+x)	//	address of this Instruction
#define V_NEXT(x) (0xfe00+x)	//	address of next Instruction
#define V_PC	0xff00			//	address of m_PC
#define V_SP	0xff01			//	address of m_SP
#define V_RNUM	0xff02			//	address of m_ReturnNum
#define V_EOF	0xffff			//	terminate code

//////////////////////////////////////////////////////////////////////////////
//	start up code
//		I don't think esp register is the same as m_SP.
//		(In i386,the stack pointer grows backword ...)

WORD i386_entrance_code[] = {

0x53,					//	push	ebx
0x51,					//	push	ecx			; for shr/shl
0x52,					//	push	edx			; for idiv(mod)

//	jump	[m_PC]
0xA1,V_PC,	 			//	mov		eax,m_PC
0x8B,0x00,				//	mov		eax,[eax]
0xFF,0xE0,				//	jmp		eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////
#if 0	//	this code is for inline,so isn't used actually

WORD i386_exit_code[] = {

0xC7,0x05,V_PC,V_NEXT(0),//	mov		m_PC,V_NEXT(0)
0x5A,					//	pop		edx
0x59,					//	pop		ecx
0x5B,					//	pop		ebx

V_EOF
};

#endif
//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_HALT[] = {

//	i386_exit_code
0xC7,0x05,V_PC,V_NEXT(0),//	mov		m_PC,V_NEXT(0)
0x5A,					//	pop		edx
0x59,					//	pop		ecx
0x5B,					//	pop		ebx

//	return 1;
0xB8,1,0,0,0,			//	mov		eax,1
0xC3,					//	ret

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_RETURN[] = {

//	pop	eax
0x83,0x2D,V_SP,0x04,	//	sub		[m_SP],4
0xA1,V_SP,				//	mov		eax,[m_SP]
0x8B,0x00,				//	mov		eax,[eax]

//	if eax==0 return 0;
0x85,0xC0,				//	test	eax,eax
0x75,14,				//	jne		skip(+14bytes)

//	i386_exit_code
0xC7,0x05,V_PC,V_NEXT(0),//	mov		m_PC,V_NEXT(0)
0x5A,					//	pop		edx
0x59,					//	pop		ecx
0x5B,					//	pop		ebx
0xC3,					//	ret

						// skip:
0x8B,0x00,				//	mov		eax,[eax]
0xFF,0xE0,				//	jmp		eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_RETURNNUM[] = {

//	pop	eax
0x83,0x2D,V_SP,0x04,	//	sub		[m_SP],4
0xA1,V_SP,				//	mov		eax,[m_SP]
0x8B,0x00,				//	mov		eax,[eax]

//	if eax==0 return 0;
0x85,0xC0,				//	test	eax,eax
0x75,14,				//	jne		skip(+14bytes)

//	i386_exit_code
0xC7,0x05,V_PC,V_NEXT(0),//	mov		m_PC,V_NEXT(0)
0x5A,					//	pop		edx
0x59,					//	pop		ecx
0x5B,					//	pop		ebx
0xc3,					//	ret

						// skip:
0x8B,0x1D,V_THIS(1),	//	mov		ebx,[V_THIS(1)]
0x89,0x1D,V_RNUM,		//	mov		V_RETNUM,ebx

0x8B,0x00,				//	mov		eax,[eax]
0xFF,0xE0,				//	jmp		eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVRETNUM[] = {

0xA1,V_RNUM,			//	mov		eax,V_RETNUM
0x8B,0x1D,V_THIS(1),	//	mov		ebx,[V_THIS(1)]
0x89,0x03,				//	mov		[ebx],eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CALL[] = {
	//	call inner procedure :
	//		but you must do a address translation
	//		 from Virtual Machine's memory map to i386 memory map
	//		 by using translation table(It's on the VM memory)

//	call	eax
//	:=	push	V_NEXT(0)
0xB8,V_NEXT(0),			//	mov		eax,V_NEXT(0)
0x8B,0x1D,V_SP,			//	mov		ebx,[m_SP]
0x89,0x03,				//	mov		[ebx],eax
0x83,0x05,V_SP,0x04,	//	add		[m_SP],4
//	+	jump	eax
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x00,				//	mov		eax,[eax]		; translate address
0xFF,0xE0,				//	jmp		eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CALLUF[] = {
	//	call user function :

0xA1,V_THIS(2),			//	mov		eax,[V_THIS(2)]	; get second argument
0x50,					//	push	eax				; is the first argument of a user function

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0xFF,0xD0,				//	call	eax				; is the address to call
0x83,0xC4,0x04,			//	add		esp,4			; adjust stack(for push eax)

0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; is the address to store a return value

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_JMP[] = {

	//	jump inner procedure :
	//		but you must do a address translation
	//		 from Virtual Machine's memory map to i386 memory map
	//		 by using translation table(It's on the VM memory)

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x00,				//	mov		eax,[eax]		; translate address
0xFF,0xE0,				//	jmp		eax


V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_XALT[] = {
	//	This is not a instruction of translation table
	//		but a instruction for a tablized jump.


0x8B,0x1D,V_THIS(1),	//	mov		ebx,[V_THIS(1)]	; get first argument
0x8B,0x04,0x9D,V_THIS(2),//	mov		eax,dword ptr [ebx*4 + V_THIS(2)]	// tablized
//	jump	[eax]
0x8B,0x00,				//	mov		eax,[eax]
0xFF,0xE0,				//	jmp		eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_PUSH[] = {

// 0xFF,0x35,V_THIS(1),	//	push	[V_THIS(1)]		; push first argument directly
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_SP,			//	mov		ebx,[m_SP]
0x89,0x03,				//	mov		[ebx],eax
0x83,0x05,V_SP,0x04,	//	add		[m_SP],4

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_PUSHM[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x00,				//	mov		eax,[eax]		;	to point
//	push eax
0x8B,0x1D,V_SP,			//	mov		ebx,[m_SP]
0x89,0x03,				//	mov		[ebx],eax
0x83,0x05,V_SP,0x04,	//	add		[m_SP],4

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_POPM[] = {

0x8B,0x1D,V_THIS(1),	//	mov		ebx,[V_THIS(1)]	; get first argument
						//							;	is the store address
0x83,0x2D,V_SP,0x04,	//	sub		[m_SP],4
0xA1,V_SP,				//	mov		eax,[m_SP]
0x8B,0x00,				//	mov		eax,[eax]
0x89,0x03,				//	mov		[ebx],eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_ADDSP[] = {

//	In i386,the stack pointer grows backword ...

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x01,0x05,V_SP,			//	add		[m_sp],eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_SUBSP[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x29,0x05,V_SP,			//	sub		[m_sp],eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_ADD[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x03,0x05,V_THIS(2),	//	add		eax,[V_THIS(2)] ; add second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_SUB[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x2B,0x05,V_THIS(2),	//	sub		eax,[V_THIS(2)] ; sub second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MUL[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x0F,0xAF,0x05,V_THIS(2),// imul	eax,[V_THIS(2)]	; multiple second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_DIV[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x99,					//	cdq
0xF7,0x3D,V_THIS(2),	//	idiv	eax,[V_THIS(2)]	; devide second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOD[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x99,					//	cdq
0xF7,0x3D,V_THIS(2),	//	idiv	eax,[V_THIS(2)]	; devide second argument
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]	; get third argument
0x89,0x10,				//	mov		[eax],edx		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOD2[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x99,					//	cdq
0xF7,0x3D,V_THIS(2),	//	idiv	eax,[V_THIS(2)]	; devide second argument
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]	; get third argument
0x89,0x10,				//	mov		[eax],edx		; store...
0x85,0xD2,				//	test	edx,edx			; but if edx<0 then
0x7D,0x08,				//  jge     skip(+8bytes)
0x03,0x15,V_THIS(2),	//	add		edx,[V_THIS(2)]	;	edx+=V_THIS(2)
0x89,0x10,				//	mov		[eax],edx		; store...
						// skip:

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_AND[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x23,0x05,V_THIS(2),	//	and		eax,[V_THIS(2)]	; bit_and to second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_OR[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x0B,0x05,V_THIS(2),	//	or		eax,[V_THIS(2)]	; bit_or to second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_XOR[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x33,0x05,V_THIS(2),	//	xor		eax,[V_THIS(2)]	; bit_xor to second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CPL[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0xF7,0xD0,				//	not		eax				; bit_compilment to second argument
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get third argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_NOT[] = {

0x33,0xC0,				//	xor		eax,eax
0x83,0x3D,V_THIS(1),0,	//	cmp		dword ptr [V_THIS(1)],0
0x0F,0x94,0xC0,			//	sete	al
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_SHR[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x0D,V_THIS(2),	//	mov		ecx,[V_THIS(2)]	; get second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0xD3,0xF8,				//	sar		eax,cl
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_SHL[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x0D,V_THIS(2),	//	mov		ecx,[V_THIS(2)]	; get second argument
0x8B,0x1D,V_THIS(3),	//	mov		ebx,[V_THIS(3)]	; get third argument
0xD3,0xE0,				//	shl		eax,cl
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_INC[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]
0xFF,0x00,				//	inc		dword ptr [eax]			; decrement

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_DEC[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]
0xFF,0x08,				//	dec		dword ptr [eax]			; decrement

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_INCS[] = {
//	increment [m_SP - Im1]

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0xF7,0xD8,				//	neg		eax
0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xFF,0x04,0x03,			//	inc		dword ptr[ebx+eax]; increment directly

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_DECS[] = {
//	decrement [m_SP - Im1]

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0xF7,0xD8,				//	neg		eax
0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xFF,0x0C,0x03,			//	dec		dword ptr[ebx+eax]; decrement directly

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_IF[] = {

//	conditional jump (for inner procedure)

0x83,0x3D,V_THIS(1),0,	//	cmp		dword ptr [V_THIS(1)],0	; if (im==0) {
0x75,9,					//	jne		skip(+9bytes)
0xA1,V_THIS(2),			//	mov		eax,[V_THIS(2)]	; get jump address
0x8B,0x00,				//	mov		eax,[eax]		; translate address
0xFF,0xE0,				//	jmp		eax
						// skip:

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOV[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get second argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVM[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x00,				//	mov		eax,[eax]		; is pointer...
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get second argument
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVMM[] = {

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get second argument
0x8B,0x1B,				//	mov		ebx,[ebx]		; is pointer...
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVS[] = {

//	[m_SP-Im2],Im1

0xA1,V_THIS(2),			//	mov		eax,[V_THIS(2)]	; get second argument
0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xF7,0xD8,				//	neg		eax
0x8B,0x0D,V_THIS(1),	//	mov		ecx,[V_THIS(1)]	; get first argument
0x89,0x0C,0x03,			//	mov		[ebx+eax],ecx	; store..

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVMS[] = {

//	mov [m_SP-Im2],[Im1]

0xA1,V_THIS(2),			//	mov		eax,[V_THIS(2)]	; get second argument
0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0x8B,0x0D,V_THIS(1),	//	mov		ecx,[V_THIS(1)]	; get first argument
0xF7,0xD8,				//	neg		eax
0x8B,0x09,				//	mov		ecx,[ecx]		; is pointer

0x89,0x0C,0x03,			//	mov		[ebx+eax],ecx	; store..

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVSS[] = {

//	mov [m_SP-Im2],[m_SP-Im1]

0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0xF7,0xD8,				//	neg		eax
0x8B,0x0C,0x03,			//	mov		ecx,[ebx+eax]	;

0xA1,V_THIS(2),			//	mov		eax,[V_THIS(2)]	; get second argument
0xF7,0xD8,				//	neg		eax
0x89,0x0C,0x03,			//	mov		[ebx+eax],ecx	; store..

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVSM[] = {

//	mov [Im2],[m_SP-Im1]

0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x15,V_THIS(2),	//	mov		edx,[V_THIS(2)]	; get second argument
0xF7,0xD8,				//	neg		eax
0x8B,0x04,0x03,			//	mov		eax,[ebx+eax]	;
0x89,0x02,				//	mov		[edx],eax		; store..

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVMSM[] = {

//	mov [Im2],[[m_SP-Im1]]

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_SP,			//	mov		ebx,[m_sp]
0xF7,0xD8,				//	neg		eax
0x8B,0x15,V_THIS(2),	//	mov		edx,[V_THIS(2)]	; get second argument
0x8B,0x04,0x03,			//	mov		eax,[ebx+eax]	;
0x8B,0x00,				//	mov		eax,[eax]		; is pointer...
0x89,0x02,				//	mov		[edx],eax		; store..

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_MOVMMM[] = {

//	mov [Im2],[[Im1]]

0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get second argument
0x8B,0x00,				//	mov		eax,[eax]		; is pointer
0x8B,0x00,				//	mov		eax,[eax]		; is pointer
0x89,0x03,				//	mov		[ebx],eax		; store...

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_LEAS[] = {

// mov [Im2],(m_SP-Im1)

0xA1,V_SP,				//	mov		eax,[m_sp]
0x2B,0x05,V_THIS(1),	//	sub		eax,[V_THIS(1)]	; get first argument
0x8B,0x1D,V_THIS(2),	//	mov		ebx,[V_THIS(2)]	; get second argument
0x89,0x03,				//	mov		[ebx],eax

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPBE[] = {

// CMP(<) Im1 Im2 lp （if (Im1 < Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x7D,0x01,				//	jge		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPLE[] = {

// CMP(<=) Im1 Im2 lp （if (Im1 <= Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x7F,0x01,				//	jg		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPAB[] = {

// CMP(>) Im1 Im2 lp （if (Im1 > Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x7E,0x01,				//	jle		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPME[] = {

// CMP(>=) Im1 Im2 lp （if (Im1 >= Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x7C,0x01,				//	jl		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPEQ[] = {

// CMP(==) Im1 Im2 lp （if (Im1 == Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x75,0x01,				//	jne		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};	//	This is not a best coding but a better.

//////////////////////////////////////////////////////////////////////////////

WORD i386_VCPU_CMPNE[] = {

// CMP(!=) Im1 Im2 lp （if (Im1 != Im2) *lp=1 else *lp=0 )

0x33,0xDB,				//	xor		ebx,ebx
0xA1,V_THIS(1),			//	mov		eax,[V_THIS(1)]	; get first argument
0x3B,0x05,V_THIS(2),	//	cmp		eax,[V_THIS(2)]	; compare
0x74,0x01,				//	je		skip(+1byte)
0x43,					//	inc		ebx
						// skip:
0xA1,V_THIS(3),			//	mov		eax,[V_THIS(3)]
0x89,0x18,				//	mov		[eax],ebx

V_EOF
};	//	This is not a best coding but a better.

//////////////////////////////////////////////////////////////////////////////
//
//		translation table
//			Virtual Machine Code to i386 Native Instruction
//
//////////////////////////////////////////////////////////////////////////////

struct {
	WORD *lpwdInstruction;
	DWORD dwSize;				//	number of arguments (DWORD)
	DWORD dwBytes;				//	size of Native Code
} VM_to_i386[] = {
	{ i386_VCPU_HALT,		0 },
	{ i386_VCPU_RETURN,		0 },
	{ i386_VCPU_RETURNNUM,	1 },
	{ i386_VCPU_MOVRETNUM,	1 },
	{ i386_VCPU_CALL,		1 },
	{ i386_VCPU_CALLUF,		3 },
	{ i386_VCPU_JMP,		1 },
	{ i386_VCPU_XALT,		2 | 256 },	//	tablized jump...
	{ i386_VCPU_PUSH,		1 },
	{ i386_VCPU_PUSHM,		1 },
	{ i386_VCPU_POPM,		1 },
	{ i386_VCPU_ADDSP,		1 },
	{ i386_VCPU_SUBSP,		1 },
	{ i386_VCPU_ADD,		3 },
	{ i386_VCPU_SUB,		3 },
	{ i386_VCPU_MUL,		3 },
	{ i386_VCPU_DIV,		3 },
	{ i386_VCPU_MOD,		3 },
	{ i386_VCPU_MOD2,		3 },
	{ i386_VCPU_AND,		3 },
	{ i386_VCPU_OR,			3 },
	{ i386_VCPU_XOR,		3 },
	{ i386_VCPU_CPL,		2 },
	{ i386_VCPU_NOT,		2 },
	{ i386_VCPU_SHR,		3 },
	{ i386_VCPU_SHL,		3 },
	{ i386_VCPU_INC,		1 },
	{ i386_VCPU_DEC,		1 },
	{ i386_VCPU_INCS,		1 },
	{ i386_VCPU_DECS,		1 },
	{ i386_VCPU_IF,			2 },
	{ i386_VCPU_MOV,		2 },
	{ i386_VCPU_MOVM,		2 },
	{ i386_VCPU_MOVMM,		2 },
	{ i386_VCPU_MOVS,		2 },
	{ i386_VCPU_MOVSM,		2 },
	{ i386_VCPU_MOVMS,		2 },
	{ i386_VCPU_MOVSS,		2 },
	{ i386_VCPU_MOVMSM,		2 },
	{ i386_VCPU_MOVMMM,		2 },
	{ i386_VCPU_LEAS,		2 },
	{ i386_VCPU_CMPBE,		3 },
	{ i386_VCPU_CMPLE,		3 },
	{ i386_VCPU_CMPAB,		3 },
	{ i386_VCPU_CMPME,		3 },
	{ i386_VCPU_CMPEQ,		3 },
	{ i386_VCPU_CMPNE,		3 },

	{ i386_entrance_code,	0 },	//	initial code(start up code)
	NULL
};

//////////////////////////////////////////////////////////////////////////////
#endif
