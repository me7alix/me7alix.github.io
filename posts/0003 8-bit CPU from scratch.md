# 8-bit CPU from scratch

![P1](https://github.com/user-attachments/assets/9df75505-df28-443a-8554-7941707fea01)

For the last 3 days I've been working on my 8-bit accumulator-based CPU ([8CPU](https://github.com/me7alix/8CPU)) in Digital Logic Simulator. My design choices aren't ideal but it's my first experience, and I think it's a good beginning.

My goal was to make a Turing-complete CPU, and I managed to achive it. To prove that my CPU is Turing complete I implemented Rule110 in my assembly. Yeah, I wrote a simple assembler for my CPU. It supports both translating code into a sequence of bytes for ROM in Digital Logic Simulator and a simulation mode for debugging.

## Registers
 - **A**: Accumulator
 - **B**: General-purpose register
 - **PC**: Program counter

## Set of instructions
 - `01` **add**:   `A = A + B`
 - `02` **sub**:   `A = A - B`
 - `03` **jmp**:   `PC = IMM`
 - `04` **jnz**:   `jmp if A != 0`
 - `05` **ldai**:  `A = RAM[B]`
 - `0F` **stai**:  `RAM[B] = A`
 - `07` **lda**:   `A = RAM[IMM]`
 - `08` **sta**:   `RAM[IMM] = A`
 - `06` **imma**:  `A = IMM`
 - `0A` **immb**:  `B = IMM`
 - `0B` **mvab**:  `B = A`
 - `09` **pra**:   `print A`
 - `0C` **scst**:  `VRAM[A] = IMM`
 - `0D` **scrf**:  `SCRN = VRAM`
 - `0E` **scrs**:  `VRAM = {0}`

## Example programs

Two numbers multiplication:

```asm
imma 8
sta 0

imma 7
sta 1

imma 0
sta 2

loop:
	lda 1
	mvab
	lda 2
	add
	sta 2

	lda 0
	immb 1
	sub
	sta 0

	jnz loop

lda 2
pra
```

Fibonacci sequence:

```asm
imma 1
sta 0
pra

imma 2
sta 1

loop:
	lda 0
	pra

	lda 1
	sta 2

	mvab
	lda 0
	add
	sta 1

	lda 2
	sta 0

	jmp loop
```
