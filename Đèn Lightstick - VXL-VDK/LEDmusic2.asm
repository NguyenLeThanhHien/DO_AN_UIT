; Define bit-addressable registers
sbit ADC0808_ALE  	= P2^0  ; Dinh nghia bit ALE cua ADC0808
sbit ADC0808_A 		= P2^1  ; Dinh nghia bit A cua ADC0808
sbit ADC0808_B 		= P2^2  ; Dinh nghia bit B cua ADC0808
sbit ADC0808_C 		= P2^3  ; Dinh nghia bit C cua ADC0808
sbit ADC0808_START	= P2^4  ; Dinh nghia bit START cua ADC0808
sbit ADC0808_EOC 	= P2^5  ; Dinh nghia bit EOC cua ADC0808
sbit ADC0808_OE 	= P2^6  ; Dinh nghia bit OE cua ADC0808
sbit ADC0808_CLK 	= P2^7  ; Dinh nghia bit CLK cua ADC0808
ADC0808_DATA 	EQU 	P3
; Define LED ports
LED_PORT_0        EQU 0x80  ; P0 - Dinh nghia cong LED PORT 0
LED_PORT_1        EQU 0x90  ; P1 - Dinh nghia cong LED PORT 1

; Define other constants
TH0_VAL EQU 0xFC           ; Gia tri khoi tao TH0
TL0_VAL EQU 0x18           ; Gia tri khoi tao TL0
MAX_HIEU_UNG EQU 5         ; Gia tri toi da cua hieu ung

; Define variables
hieu_ung DATA 0x20         ; Bien luu tru hieu ung
t DATA 0x21                ; Bien dem thoi gian
adc_values DATA 0X22       ; Bien luu ket qua doc tu ADC
position DATA 0x23         ; Bien luu vi tri

; Define subroutines
XUNG:
    CLR ADC0808_CLK        ; Xoa bit CLK
    SETB ADC0808_CLK       ; Set bit CLK
    RET                    ; Return

ADC0808_READ:
    SETB ADC0808_A
	SETB ADC0808_B
	SETB ADC0808_C
	CLR ADC0808_ALE
	CLR ADC0808_START
	SETB ADC0808_ALE
	SETB ADC0808_START
	CLR ADC0808_ALE
	CLR ADC0808_START
	JNB ADC0808_EOC, $
	SETB ADC0808_OE
	MOV adc_values, ADC0808_DATA

DELAY:
    MOV R0, #00H          ; Khoi tao R0
DELAY_LOOP1:
    MOV R1, #00H          ; Khoi tao R1
DELAY_LOOP2:
    MOV R2, #7BH          ; Khoi tao R2
DELAY_LOOP3:
    DJNZ R2, DELAY_LOOP3   ; Giam R2, neu chua ve 0 thi lap lai
    DJNZ R1, DELAY_LOOP2   ; Giam R1, neu chua ve 0 thi lap lai
    DJNZ R0, DELAY_LOOP1   ; Giam R0, neu chua ve 0 thi lap lai
    RET                    ; Return

ISR_TIMER0:
    INC t                  ; Tang bien dem thoi gian t
    MOV TH0, #TH0_VAL      ; Khoi tao lai TH0
    MOV TL0, #TL0_VAL      ; Khoi tao lai TL0
    SETB TR0               ; Bat Timer0
    MOV A, t               ; Chuyen gia tri cua t vao A
    CJNE A, #10000, NOT_MAX_HIEU_UNG ; So sanh gia tri A voi 10000
    MOV t, #0              ; Neu bang 10000 thi reset t ve 0
    INC hieu_ung           ; Tang bien hieu ung
    MOV A, hieu_ung        ; Chuyen gia tri hieu_ung vao A
    CJNE A, #MAX_HIEU_UNG, NOT_MAX_HIEU_UNG ; So sanh hieu_ung voi MAX_HIEU_UNG
    MOV hieu_ung, #0       ; Neu bang MAX_HIEU_UNG thi reset hieu_ung ve 0
NOT_MAX_HIEU_UNG:
    RETI                   ; Return from interrupt

MAIN:
    MOV IE, #0x82          ; Bat ngat Timer0 va ngat toan cuc
    MOV TMOD, #0x01        ; Chon che do 1 cho Timer0
    MOV TH0, #TH0_VAL      ; Khoi tao gia tri TH0
    MOV TL0, #TL0_VAL      ; Khoi tao gia tri TL0
    SETB TR0               ; Bat Timer0

    MOV hieu_ung, #0       ; Khoi tao hieu_ung ve 0
    MOV t, #0              ; Khoi tao t ve 0
    MOV P0, #0FFH          ; Tat het cac den o PORT0
    MOV P1, #0FFH          ; Tat het cac den o PORT1
    MOV P2, #0FFH          ; Tat het cac den o PORT2

MAIN_LOOP:
    MOV A, hieu_ung        ; Chuyen gia tri hieu_ung vao A
    ;CJNE A, #0, CHECK_CASE1 ; Neu khong phai truong hop 0 thi kiem tra truong hop 1
    ; Case 0: LED sang tu thap len cao
    ACALL ADC0808_READ     ; Goi ham doc gia tri tu ADC
    MOV A, adc_values      ; Chuyen gia tri adc_values vao A
    CJNE A, #12, CASE0_ELSE1 ; Neu gia tri <= 12 thi tat het den
    MOV P0, #00H
    MOV P1, #00H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE1:
    CJNE A, #18, CASE0_ELSE2 ; Neu gia tri <= 18 thi bat mot den
    MOV P0, #80H
    MOV P1, #00H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE2:
    CJNE A, #24, CASE0_ELSE3 ; Neu gia tri <= 24 thi bat nhieu den hon
    MOV P0, #0C0H
    MOV P1, #00H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE3:
    CJNE A, #30, CASE0_ELSE4 ; Neu gia tri <= 36 thi bat nhieu den hon
    MOV P0, #0E0H          
    MOV P1, #00H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE4:
    CJNE A, #36, CASE0_ELSE5 ; Neu gia tri <= 36 thi bat nhieu den hon
    MOV P0, #0F0H          
    MOV P1, #000H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE5:
    CJNE A, #42, CASE0_ELSE6 ; Neu gia tri <= 42 thi bat nhieu den hon
    MOV P0, #0F8H          
    MOV P1, #000H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE6:
    CJNE A, #48, CASE0_ELSE7 ; Neu gia tri <= 48 thi bat nhieu den hon
    MOV P0, #0FCH          
    MOV P1, #000H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE7:
    CJNE A, #54, CASE0_ELSE8 ; Neu gia tri <= 54 thi bat nhieu den hon
    MOV P0, #0FEH          
    MOV P1, #000H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE8:
    CJNE A, #60, CASE0_ELSE9 ; Neu gia tri <= 60 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #000H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE9:
    CJNE A, #66, CASE0_ELSE10 ; Neu gia tri <= 66 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #80H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE10:
    CJNE A, #72, CASE0_ELSE11 ; Neu gia tri <= 72 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #0xC0
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE11:
    CJNE A, #78, CASE0_ELSE12 ; Neu gia tri <= 78 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #0xE0
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE12:
    CJNE A, #84, CASE0_ELSE13 ; Neu gia tri <= 84 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #0F0H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE13:
    CJNE A, #90, CASE0_ELSE14 ; Neu gia tri <= 90 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #0F8H
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE14:
    CJNE A, #96, CASE0_ELSE16 ; Neu gia tri <= 96 thi bat nhieu den hon
    MOV P0, #0FFH          
    MOV P1, #0FCH
	ACALL DELAY
	MOV P1, #0FEH
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
;CASE0_ELSE15:
;    CJNE A, #96, CASE0_ELSE16 ; Neu gia tri <= 36 thi bat nhieu den hon
;    MOV P0, #0FFH          
;    MOV P1, #0FEH
;    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh
CASE0_ELSE16:
    MOV P0, #0FFH          
    MOV P1, #0FFH
    LJMP MAIN_LOOP_END     ; Nhay ve cuoi vong lap chinh

CHECK_CASE1:
    MOV A, hieu_ung        	; Chuyen gia tri hieu_ung vao A
    CJNE A, #1, CHECK_CASE2 ; Neu khong phai truong hop 1 thi kiem tra truong hop 2
    ; Case 1: LED sang dam vao nhau
    ACALL ADC0808_READ     	; Goi ham doc gia tri tu ADC
    MOV A, adc_values      	; Chuyen gia tri adc_values vao A
    CJNE A, #12, CASE1_ELSE1 
    MOV P0, #00H
	MOV P1, #00H
    LJMP MAIN_LOOP_END     	
CASE1_ELSE1:
    CJNE A, #24, CASE1_ELSE2  
    MOV P1, #01H
	MOV P0, #80H
    LJMP MAIN_LOOP_END     	 
CASE1_ELSE2:
    CJNE A, #36, CASE1_ELSE3  
    MOV P0, #03H
	MOV P1, #0C0H
    LJMP MAIN_LOOP_END      
CASE1_ELSE3:
    CJNE A, #48, CASE1_ELSE4  
    MOV P0, #07H
	MOV P1, #0E0H           
    LJMP MAIN_LOOP_END      
CASE1_ELSE4:
    CJNE A, #60, CASE1_ELSE5  
    MOV P0, #0FH
	MOV P1, #0F0H           
    LJMP MAIN_LOOP_END      
CASE1_ELSE5:
    CJNE A, #72, CASE1_ELSE6 
    MOV P0, #1FH
	MOV P1, #0F8H           
    LJMP MAIN_LOOP_END      
CASE1_ELSE6:
    CJNE A, #84, CASE1_ELSE7  
    MOV P0, #3FH
	MOV P1, #0FCH          
    LJMP MAIN_LOOP_END      
CASE1_ELSE7:
    CJNE A, #96, CASE1_ELSE8  
    MOV P0, #7FH
	MOV P1, #0FEH           
    LJMP MAIN_LOOP_END      
CASE1_ELSE8:
    MOV P0, #0FFH
	MOV P1, #0FFH           
    LJMP MAIN_LOOP_END      
	
CHECK_CASE2:
    MOV A, hieu_ung        	; Chuyen gia tri hieu_ung vao A
    CJNE A, #2, CHECK_CASE3 ; Neu khong phai truong hop 2 thi kiem tra truong hop 3
    ; Case 2: LED sang dam vao nhau (nguoc lai)
    ACALL ADC0808_READ     	; Goi ham doc gia tri tu ADC
    MOV A, adc_values      	; Chuyen gia tri adc_values vao A
    CJNE A, #12, CASE2_ELSE1 ; Neu gia tri <= 100 thi tat mot den
    MOV P0, #00H
	MOV P1, #00H
    LJMP MAIN_LOOP_END     	
CASE2_ELSE1:
    CJNE A, #24, CASE2_ELSE2 
    MOV P1, #80H
	MOV P0, #01H
    LJMP MAIN_LOOP_END     	
CASE2_ELSE2:
    CJNE A, #36, CASE2_ELSE3 
    MOV P0, #0C0H
	MOV P1, #03H
    LJMP MAIN_LOOP_END     
CASE2_ELSE3:
    CJNE A, #48, CASE2_ELSE4 ;
    MOV P0, #0E0H
	MOV P1, #07H          
    LJMP MAIN_LOOP_END     
CASE2_ELSE4:
    CJNE A, #60, CASE2_ELSE5 ;
    MOV P0, #0F0H
	MOV P1, #0FH          
    LJMP MAIN_LOOP_END     
CASE2_ELSE5:
    CJNE A, #72, CASE1_ELSE6 
    MOV P0, #0F8H
	MOV P1, #1FH          
    LJMP MAIN_LOOP_END    
CASE2_ELSE6:
    CJNE A, #84, CASE2_ELSE7 
    MOV P0, #0FCH
	MOV P1, #3FH          
    LJMP MAIN_LOOP_END     
CASE2_ELSE7:
    CJNE A, #96, CASE2_ELSE8 
    MOV P0, #0FEH
	MOV P1, #7FH          ;
    LJMP MAIN_LOOP_END     
CASE2_ELSE8:
    MOV P0, #0FFH
	MOV P1, #0FFH          
    LJMP MAIN_LOOP_END     
	
CHECK_CASE3:
    MOV A, hieu_ung        	; Chuyen gia tri hieu_ung vao A
    CJNE A, #3, CHECK_CASE4 ; Neu khong phai truong hop 3 thi kiem tra truong hop 4
    ; Case 3: Sang tung cum 4 bong
    ACALL ADC0808_READ     	; Goi ham doc gia tri tu ADC
    MOV A, adc_values      	; Chuyen gia tri adc_values vao A
    CJNE A, #24, CASE3_ELSE1 
    MOV P0, #0F0H
	MOV P1, #0FH
    LJMP MAIN_LOOP_END     	
CASE3_ELSE1:
    CJNE A, #46, CASE3_ELSE2 
    MOV P1, #0FH
	MOV P0, #0F0H
    LJMP MAIN_LOOP_END     	
CASE3_ELSE2:
    CJNE A, #60, CASE3_ELSE3 
    MOV P0, #0FH
	MOV P1, #0FH
    LJMP MAIN_LOOP_END     
CASE3_ELSE3:
    CJNE A, #84, CASE3_ELSE4 
    MOV P0, #0F0H
	MOV P1, #0F0H          
    LJMP MAIN_LOOP_END     
CASE3_ELSE4:
    MOV P0, #0FFH
	MOV P1, #0FFH          
    LJMP MAIN_LOOP_END     

CHECK_CASE4:
    MOV A, hieu_ung        	
    CJNE A, #4, MAIN_LOOP_END 
    ; Case 3: Sang xen ke
    ACALL ADC0808_READ     	
    MOV A, adc_values      	
    CJNE A, #24, CASE4_ELSE1 
    MOV P0, #00H
	MOV P1, #00H
    LJMP MAIN_LOOP_END     	
CASE4_ELSE1:
    CJNE A, #46, CASE4_ELSE2 
    MOV P1, #18H
	MOV P0, #81H
    LJMP MAIN_LOOP_END     	
CASE4_ELSE2:
    CJNE A, #60, CASE4_ELSE3 
    MOV P0, #0AAH
	MOV P1, #0AAH
    LJMP MAIN_LOOP_END     
CASE4_ELSE3:
    CJNE A, #84, CASE4_ELSE4 
    MOV P0, #55H
	MOV P1, #55H          
    LJMP MAIN_LOOP_END     
CASE4_ELSE4:
    MOV P0, #0FFH
	MOV P1, #0FFH          
    LJMP MAIN_LOOP_END     

MAIN_LOOP_END:
    LJMP MAIN_LOOP         
END                        
