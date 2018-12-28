from scipy.misc import imread

img = imread('../assets/bbc.bmp')

(height, width, colours) = img.shape

with open('bbc.h', 'w') as f:
	f.write('static const uint8_t bbc[' + str(height) + ' * ' + str(width) + ' * ' + str(colours) + '] = \r\n{')
	for r in range(height):
		for c in range(width):
			for col in reversed(img[r][c]):
				f.write(str(col) + ', ')
		f.write('\r\n')
	f.write('};')