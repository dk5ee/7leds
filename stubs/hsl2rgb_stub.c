// HSL to RGB

void setRGB(uint8_t R,uint8_t G,uint8_t B) {
	...
}

float hue2rgb(float p,float q,float t) {
	while (t<0) t+=1.0;
	while (t>1.0) t-=1.0;
	    if(t < 1/6) return p + (q - p) * 6 * t;
            if(t < 1/2) return q; 
            if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
	
}

void setHSL(float h,float s,float l) {
	while (h<0.0) h+=1.0;
	while (h>1.0) h-=1.0;

	if (s>1.0) s=1.0;
	if (l<0.0) l=0.0;
	if (l>1.0) l=1.0;
	
	
	uint8_t R;
	uint8_t G;
	uint8_t B;
	
	if(s =< 0){
		R = 255 *l;
		G = R;
		B = R;
	else {
		float q= (l<0.5)? l*(1+s) : l+s - l*s;
		float p= 2*l -q;
		R = 255 * hue2rgb (p,q, h+ 1/3);
		G = 255 * hue2rgb (p,q, h);
		B = 255 * hue2rgb (p,q, h- 1/3);
	}
	setRBG( R,G, B);
}
	