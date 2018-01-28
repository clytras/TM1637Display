
#define MAKELONG(hi, low) (((long) hi) << 16 | (low))
#define HIWORD(w) ((w) >> 16)
#define LOWORD(w) ((w) & 0xffff)

uint8_t sizeofsz(char* ch) {
	int tmp = 0;
	while (*ch) {
		*ch++;
		tmp++;
	}
	return tmp;
}
