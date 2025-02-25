#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	fprintf(stdout, "output_format_echo\n");
	fflush(stdout);

	int8_t i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;

	uint8_t len;
	char buf[256];

	uint8_t type;
	while (fread(&type, sizeof(type), 1, stdin) == 1) {
		switch (type) {
		case 0:
			fread(&i8, sizeof(i8), 1, stdin);
			fprintf(stdout, "i8=%d\n", i8);
			break;
		case 1:
			fread(&i16, sizeof(i16), 1, stdin);
			fprintf(stdout, "i16=%d\n", i16);
			break;
		case 2:
			fread(&i32, sizeof(i32), 1, stdin);
			fprintf(stdout, "i32=%d\n", i32);
			break;
		case 3:
			fread(&i64, sizeof(i64), 1, stdin);
			fprintf(stdout, "i64=%d\n", i64);
			break;
		case 4:
			fread(&u8, sizeof(u8), 1, stdin);
			fprintf(stdout, "u8=%u\n", u8);
			break;
		case 5:
			fread(&u16, sizeof(u16), 1, stdin);
			fprintf(stdout, "u16=%u\n", u16);
			break;
		case 6:
			fread(&u32, sizeof(u32), 1, stdin);
			fprintf(stdout, "u32=%u\n", u32);
			break;
		case 7:
			fread(&u64, sizeof(u64), 1, stdin);
			fprintf(stdout, "u64=%u\n", u64);
			break;
		case 8:
			fread(&len, sizeof(len), 1, stdin);
			fread(buf, 1, len, stdin);
			buf[len] = '\0';
			fprintf(stdout, "b(true)=%s\n", buf);
			break;
		case 9:
			fread(&len, sizeof(len), 1, stdin);
			fread(buf, 1, len, stdin);
			buf[len] = '\0';
			fprintf(stdout, "b(false)=%s\n", buf);
			break;
		case 10:
			fread(&len, sizeof(len), 1, stdin);
			fread(buf, 1, len, stdin);
			buf[len] = '\0';
			fprintf(stdout, "s=%s\n", buf);
			break;
		default:
			fprintf(stdout, "ERROR: unknown type %d\n", type);
			break;
		}
		fflush(stdout);
	}
	return 0;
}
