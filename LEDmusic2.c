#include <REGX51.H>

// Dinh nghia cac chan dieu khien cua ADC0808
#define ADC0808_DATA      P3   // Chan du lieu ADC0808 ket noi voi cong P3
#define ADC0808_A         P2_1 // Chan dia chi A cua ADC0808 ket noi voi chan P2_1
#define ADC0808_B         P2_2 // Chan dia chi B cua ADC0808 ket noi voi chan P2_2
#define ADC0808_C         P2_3 // Chan dia chi C cua ADC0808 ket noi voi chan P2_3
#define ADC0808_ALE       P2_0 // Chan ALE cua ADC0808 ket noi voi chan P2_0
#define ADC0808_START     P2_4 // Chan START cua ADC0808 ket noi voi chan P2_4
#define ADC0808_EOC       P2_5 // Chan EOC cua ADC0808 ket noi voi chan P2_5
#define ADC0808_OE        P2_6 // Chan OE cua ADC0808 ket noi voi chan P2_6
#define ADC0808_CLK       P2_7 // Chan CLK cua ADC0808 ket noi voi chan P2_7
#define LED_PORT_0        P0   // Cong LED_PORT_0 ket noi voi cong P0
#define LED_PORT_1        P1   // Cong LED_PORT_1 ket noi voi cong P1

int hieu_ung = 0; // Bien hieu ung ban dau
int t = 0;       // Bien dem thoi gian ban dau

// Ham xung tao xung clock cho ADC0808
void xung() {
    ADC0808_CLK = 0; // Tao xung clock 0
    ADC0808_CLK = 1; // Tao xung clock 1
}

// Ham doc gia tri tu ADC0808
unsigned char ADC0808_Read() {
    unsigned char kq;         // Bien luu gia tri ket qua doc tu ADC0808
    ADC0808_START = 1;        // Bat dau qua trinh lay mau
    xung();                   // Tao xung clock
    ADC0808_A = 0;            // Dia chi A la 0
    ADC0808_B = 0;            // Dia chi B la 0
    ADC0808_C = 0;            // Dia chi C la 0
    ADC0808_ALE = 1;          // Xac nhan dia chi
    xung();                   // Tao xung clock
    ADC0808_ALE = 0;          // Ket thuc xac nhan dia chi
    xung();                   // Tao xung clock
    ADC0808_START = 0;        // Ket thuc qua trinh lay mau
    xung();                   // Tao xung clock
    ADC0808_EOC = 1;          // Kiem tra EOC, bat dau qua trinh chuyen doi
    while (ADC0808_EOC == 0) { // Cho den khi EOC bang 0
        xung();               // Tao xung clock
    }
    ADC0808_OE = 1;           // Cho phep doc du lieu
    kq = ADC0808_DATA;        // Lay ket qua du lieu tu ADC0808
    ADC0808_OE = 0;           // Ket thuc qua trinh doc du lieu
    return kq;                // Tra ve gia tri ket qua
}

// Ham delay tao do tre
void delay(unsigned int t) {
    unsigned int i, j;        // Bien dem i va j
    for (i = 0; i < t; ++i) { // Vong lap ngoai
        for (j = 0; j < 123; ++j) { // Vong lap trong
            // Noi dung vong lap trong de tao do tre
        }
    }
}

// Chuong trinh ngat cho timer 0
void ngat() interrupt 1 // Ngat timer 0
{
    t++;               // Tang bien dem thoi gian
    TH0 = 0xFC;        // Thiet lap gia tri cho TH0 de tao khoang tre 1ms
    TL0 = 0x18;        // Thiet lap gia tri cho TL0
    TR0 = 1;           // Khoi dong bo dinh thoi
    if (t >= 10000) {  // Neu bien dem thoi gian lon hon hoac bang 10000
        t = 0;         // Reset bien dem thoi gian ve 0
        hieu_ung = (hieu_ung + 1) % 3; // Tang bien hieu ung va lay phan du cho 3
    }
}

// Chuong trinh chinh
void main(void) {
    unsigned char adc_values; // Bien luu gia tri doc tu ADC
    LED_PORT_0 = LED_PORT_1 = 0xFF; // Tat het cac den LED
    P2 = 0xFF;               // Tat het cac den tren cong P2

    TMOD = 0x01;             // Timer 0 che do 1
    ET0 = 1;                 // Kich hoat timer 0 cho phep ngat
    TR0 = 1;                 // Bat timer 0
    EA = 1;                  // Cho phep ngat toan cuc

    while (1) {              // Vong lap vo han
        switch (hieu_ung) {  // Kiem tra gia tri hieu_ung
            case 0: {        // Truong hop hieu_ung = 0
                adc_values = ADC0808_Read(); // Doc gia tri tu ADC0808

                if (adc_values <= 100) {      // Neu gia tri adc <= 100
                    LED_PORT_0 = 0x00;        // Tat LED tren PORT 0
                    LED_PORT_1 = 0x00;        // Tat LED tren PORT 1
                } else if (adc_values <= 118) { // Neu gia tri adc <= 118
                    LED_PORT_0 = 0x01;        // Bat LED thu nhat tren PORT 0
                    LED_PORT_1 = 0x80;        // Bat LED thu nhat tren PORT 1
                } else if (adc_values <= 136) { // Neu gia tri adc <= 136
                    LED_PORT_0 = 0x03;        // Bat LED thu nhat va thu hai tren PORT 0
                    LED_PORT_1 = 0xC0;        // Bat LED thu nhat va thu hai tren PORT 1
                } else if (adc_values <= 154) { // Neu gia tri adc <= 154
                    LED_PORT_0 = 0x07;        // Bat LED thu nhat, thu hai va thu ba tren PORT 0
                    LED_PORT_1 = 0xE0;        // Bat LED thu nhat, thu hai va thu ba tren PORT 1
                } else if (adc_values <= 170) { // Neu gia tri adc <= 170
                    LED_PORT_0 = 0x0F;        // Bat LED thu nhat den thu tu tren PORT 0
                    LED_PORT_1 = 0xF0;        // Bat LED thu nhat den thu tu tren PORT 1
                } else if (adc_values <= 188) { // Neu gia tri adc <= 188
                    LED_PORT_0 = 0x1F;        // Bat LED thu nhat den thu nam tren PORT 0
                    LED_PORT_1 = 0xF8;        // Bat LED thu nhat den thu nam tren PORT 1
                } else if (adc_values <= 206) { // Neu gia tri adc <= 206
                    LED_PORT_0 = 0x3F;        // Bat LED thu nhat den thu sau tren PORT 0
                    LED_PORT_1 = 0xFC;        // Bat LED thu nhat den thu sau tren PORT 1
                } else if (adc_values <= 224) { // Neu gia tri adc <= 224
                    LED_PORT_0 = 0x7F;        // Bat LED thu nhat den thu bay tren PORT 0
                    LED_PORT_1 = 0xFE;        // Bat LED thu nhat den thu bay tren PORT 1
                } else {
                    LED_PORT_0 = 0xFF;        // Bat tat ca LED tren PORT 0
                    LED_PORT_1 = 0xFF;        // Bat tat ca LED tren PORT 1
                }
                break;
            }
            case 1: {        // Truong hop hieu_ung = 1
                adc_values = ADC0808_Read(); // Doc gia tri tu ADC0808

                if (adc_values <= 118) {      // Neu gia tri adc <= 118
                    LED_PORT_0 = 0xF0;        // Bat 4 LED tren PORT 0
                    LED_PORT_1 = 0x0F;        // Bat 4 LED tren PORT 1
                    delay(500);               // Tao do tre 500
                } else if (adc_values <= 154) { // Neu gia tri adc <= 154
                    LED_PORT_0 = 0x0F;        // Bat 4 LED khac tren PORT 0
                    LED_PORT_1 = 0xF0;        // Bat 4 LED khac tren PORT 1
                    delay(500);               // Tao do tre 500
                } else if (adc_values <= 188) { // Neu gia tri adc <= 188
                    LED_PORT_0 = 0x0F;        // Bat 4 LED tren PORT 0
                    LED_PORT_1 = 0x0F;        // Bat 4 LED tren PORT 1
                    delay(500);               // Tao do tre 500
                } else if (adc_values <= 224) { // Neu gia tri adc <= 224
                    LED_PORT_0 = 0xF0;        // Bat 4 LED khac tren PORT 0
                    LED_PORT_1 = 0xF0;        // Bat 4 LED khac tren PORT 1
                    delay(500);               // Tao do tre 500
                } else {
                    LED_PORT_0 = 0xFF;        // Bat tat ca LED tren PORT 0
                    LED_PORT_1 = 0xFF;        // Bat tat ca LED tren PORT 1
                    delay(500);               // Tao do tre 500
                }
                break;
            }
            case 2: {        // Truong hop hieu_ung = 2
                adc_values = ADC0808_Read(); // Doc gia tri tu ADC0808

                if (adc_values <= 100) {      // Neu gia tri adc <= 100
                    LED_PORT_0 = 0x00;        // Tat LED tren PORT 0
                    LED_PORT_1 = 0xFF;        // Bat tat ca LED tren PORT 1
                } else if (adc_values <= 118) { // Neu gia tri adc <= 118
                    LED_PORT_0 = 0x01;        // Bat LED thu nhat tren PORT 0
                    LED_PORT_1 = 0xFE;        // Bat 7 LED khac tren PORT 1
                } else if (adc_values <= 136) { // Neu gia tri adc <= 136
                    LED_PORT_0 = 0x03;        // Bat LED thu nhat va thu hai tren PORT 0
                    LED_PORT_1 = 0xFC;        // Bat 6 LED khac tren PORT 1
                } else if (adc_values <= 154) { // Neu gia tri adc <= 154
                    LED_PORT_0 = 0x07;        // Bat LED thu nhat, thu hai va thu ba tren PORT 0
                    LED_PORT_1 = 0xF8;        // Bat 5 LED khac tren PORT 1
                } else if (adc_values <= 170) { // Neu gia tri adc <= 170
                    LED_PORT_0 = 0x0F;        // Bat LED thu nhat den thu tu tren PORT 0
                    LED_PORT_1 = 0xF0;        // Bat 4 LED khac tren PORT 1
                } else if (adc_values <= 188) { // Neu gia tri adc <= 188
                    LED_PORT_0 = 0x1F;        // Bat LED thu nhat den thu nam tren PORT 0
                    LED_PORT_1 = 0xE0;        // Bat 3 LED khac tren PORT 1
                } else if (adc_values <= 206) { // Neu gia tri adc <= 206
                    LED_PORT_0 = 0x3F;        // Bat LED thu nhat den thu sau tren PORT 0
                    LED_PORT_1 = 0xC0;        // Bat 2 LED khac tren PORT 1
                } else if (adc_values <= 224) { // Neu gia tri adc <= 224
                    LED_PORT_0 = 0x7F;        // Bat LED thu nhat den thu bay tren PORT 0
                    LED_PORT_1 = 0x80;        // Bat 1 LED tren PORT 1
                } else {
                    LED_PORT_0 = 0xFF;        // Bat tat ca LED tren PORT 0
                    LED_PORT_1 = 0x00;        // Tat LED tren PORT 1
                }
                break;
            }
        }
    }
}
