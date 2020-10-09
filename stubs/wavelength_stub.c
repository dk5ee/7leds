uint16_t ledWL[] =
{ 
	700, //highest wavelength, no LED
	660, //deep red		0
	620, //red			1
	590, //yellow			2
	520, //emerald green	3
	465, //bright blue		4
	445, //royal blue		5
	400, //violet			6
	350, //lowest wavelength, no LED
}

void setleds(uint8_t led, uint8_t value) {
	/*
	..
	*/
}
void ledsbywavelength (uint16_t wavelength, uint8_t intensity) {
	for (uint8_t led = 0; led<7; led++) {
		uint16_t thisledWL = ledWL[led+1];
		if (thisledWL ==wavelength) {
			setleds(led, intensity);
		} else if ((thisledWL < wavelength) {
			uint16_t lowerledWL =  ledWL[led];
			if (wavelength > lowerledWL) {
				int16_t difference =  thisledWL-lowerledWL;
				int16_t vicinity = wavelength-lowerledWL;
				int16_t newintensity = (intensity * vicinity )/difference;
				setleds(led, newintensity);
			}else {
				//distance too far
				setleds(led, 0);
			}
		} else {
			uint16_t upperledWL =  ledWL[led+2];
			if (wavelength < upperledWL) {
				int16_t difference =  upperledWL -thisledWL;
				int16_t vicinity = upperledWL-wavelength;
				int16_t newintensity = (intensity * vicinity )/difference;
				setleds(led, newintensity);
			}else {
				//distance too far
				setleds(led, 0);
			}
		}
	}
}

