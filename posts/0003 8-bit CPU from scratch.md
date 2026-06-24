# 8-bit CPU from scratch

![P1](https://github.com/user-attachments/assets/620ae46c-2f94-48f6-afc7-2ee7c7f0beac)

For the last 3 days I've been working on my 8-bit accumulator-based CPU [(8CPU)](https://github.com/me7alix/8CPU) in Digital Logic Simulator. My design choices aren't ideal but it's my first experience, and I think it's a good beginning.

My goal was to make a Turing-complete CPU, and I managed to achive it. To prove that my CPU is Turing complete I implemented Rule110 in my assembly. Yeah, I wrote a simple assembler for my CPU. It supports both translating code into a sequence of bytes for ROM in Digital Logic Simulator and a simulation mode for debugging.

## Registers
 - **PCTR**: Program counter
 - **A**: Accumulator
 - **B**: General-purpose register

## Set of instructions
 - `01` **add**:   `A = A + B`
 - `02` **sub**:   `A = A - B`
 - `03` **jmp**:   `PCTR = IMM`
 - `04` **jnz**:   `jmp if A != 0`
 - `05` **jne**:   `jmp if A != B`
 - `06` **imma**:  `A = IMM`
 - `07` **lda**:   `A = RAM[B]`
 - `08` **sta**:   `RAM[B] = A`
 - `09` **prnta**: `print A`
 - `0A` **immb**:  `B = IMM`
 - `0B` **movb**:  `B = A`
 - `0C` **scst**:  `BRAM[A] = IMM`
 - `0D` **scrf**:  `VRAM = BRAM`
 - `0E` **scrs**:  `BRAM = {0}`

## Example programs

Two numbers multiplication:

```asm
imma 8 ; A
sta 0

imma 7 ; B
sta 1

imma 0
sta 2

loop:
    lda 1
    movb
    lda 2
    add
    sta 2

    lda 0
    immb 1
    sub
    sta 0

    jnz loop

lda 2
prnta
```

Fibonacci sequence:

```asm
imma 1
sta 0
prnta

imma 2
sta 1

loop:
    lda 0
    prnta

    lda 1
    sta 2

    movb
    lda 0
    add
    sta 1

    lda 2
    sta 0

    jmp loop
```
