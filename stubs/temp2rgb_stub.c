// this algorithmus wanders through the net.. only 3 colours

typedef struct RgbColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RgbColor;



void temp2rgb(float temp) {
	float R,G,B;
	if (temp < 6600.0) {
		R = 1;
	} else {
		
	R = 1.292936186 * pow(((temp/100)-60.0), -0.1332047592)
	If (R < 0.0) R = 0.0;
	If (R > 1.0) R = 1.0;
	}
	
	if (temp <= 6600.0) {
	
		G = 0.3900815787 * log(temp/100) - 0.6318414437;
		If (G < 0.0) G = 0.0;
		If (G > 1.0) G = 1.0;
	} else {
		
G  = 1.1298908608 * pow(((temp/100)-60.0),-0.0755148492);
		If (G < 0.0) G = 0.0;
		If (G > 1.0) G = 1.0;
	}
	
	if (temp >= 6600.0) {
		B = 1.0;
	} elseif {temp<=1900) {
		B = 0.0;
	} else {
		Blue = Temperature - 10
		Blue = 0.5432067891 * log(((temp/100)-10.0)) - 1.1962540891;
		If (B < 0.0) B = 0.0;
		If (B > 1.0) B = 1.0;
	}
}