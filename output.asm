.import print
.import init
.import new
.import delete

lis $4
.word 4
lis $11
.word 1
sw $31, -4($30)
sub $30, $30, $4

sw $2, -4($30)
sub $30, $30, $4
add $2, $0, $0
lis $3
.word init
jalr $3
add $30, $30, $4
lw $2, -4($30)
add $30, $30, $4
lw $31, -4($30)
sw $31, -4($30)
sub $30, $30, $4
lis $3
.word wain
jalr $3
add $30, $30, $4
lw $31, -4($30)
jr $31
funcnoArgs:
add $29, $0, $30
lis $3
.word 123
; procedure pop
lis $5
.word 0
add $30, $30, $5
jr $31
wain: 
sw $31, -4($30)
sub $30, $30, $4
add $29, $30, $0
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4

; add
sw $5, -4($30)
sub $30, $30, $4
; add
sw $5, -4($30)
sub $30, $30, $4
sw $31, -4($30)
sub $30, $30, $4
lis $3
.word funcnoArgs
jalr $3
add $30, $30, $4
lw $31, -4($30)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
add $30, $30, $4
lw $5, -4($30)

sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
add $30, $30, $4
lw $5, -4($30)

lis $5
.word 8
add $30, $30, $5
add $30, $30, $4
lw $31, -4($30)
jr $31
