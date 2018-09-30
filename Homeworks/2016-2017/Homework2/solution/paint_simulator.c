#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEIGHT_MAX 1024
#define WIDTH_MAX  1024

#define EPERM  1
#define EINVAL 2
#define ENOMEM 3

#define SWAP(lhs, rhs) \
	do { \
		typeof(*lhs) tmp = *lhs; \
		*lhs = *rhs; \
		*rhs = tmp; \
	} while (0)

static int check_arg_bounds(size_t param, size_t lower, size_t upper) {

    if (param < lower || param >= upper) {
        fprintf(stderr, "%d\n", EINVAL);
        return 0;
    }
    return 1;
}

struct pixel {
	unsigned char r, g, b;
};

struct image {
	size_t width;
	size_t height;
	struct pixel **pixels;
};

enum OP_TYPES {
	OP_INIT = 1,
	OP_CROP,
	OP_RESIZE,
	OP_COLOR,
	OP_BLUR,
	OP_ROTATE,
	OP_FILL,
	OP_PRINT
};

static void new_image(struct image *img, size_t width, size_t height)
{
	img->width = width;
	img->height = height;
	img->pixels = malloc(img->height * sizeof(*img->pixels));
	assert(img->pixels != NULL);
	for (size_t i = 0; i < img->height; ++i) {
		img->pixels[i] = malloc(img->width * sizeof(*img->pixels[i]));
		assert(img->pixels[i] != NULL);
	}
}

static void copy_image(struct image *img, struct image *img_copy)
{
	new_image(img_copy, img->width, img->height);
	for (size_t i = 0; i < img_copy->height; ++i)
		memcpy(img_copy->pixels[i], img->pixels[i],
		       img_copy->width * sizeof(*img->pixels[i]));
}

static void read_pixel(struct pixel *pix)
{
	scanf("%hhu%hhu%hhu", &pix->r, &pix->g, &pix->b);
}

static int init_image(struct image *img, size_t width, size_t height)
{
	assert(img != NULL);
	new_image(img, width, height);

	for (size_t i = 0; i < img->height; ++i)
		for (size_t j = 0; j < img->width; ++j)
			read_pixel(&img->pixels[i][j]);

	return 0;
}

static void free_image(struct image *img, unsigned char init)
{
    if (!init) {
        return;
    }

	for (size_t i = 0; i < img->height; ++i)
		free(img->pixels[i]);
	free(img->pixels);
}

static int crop_image(struct image *img, size_t start_x, size_t end_x,
		      size_t start_y, size_t end_y)
{
	/* Update width and height */
	size_t new_width = end_y - start_y + 1;
	size_t new_height = end_x - start_x + 1;

	/* Shift submatrix in origin */
	for (size_t i = 0; i < new_height; ++i)
		for (size_t j = 0; j < new_width; ++j)
			img->pixels[i][j] =
				img->pixels[i + start_x][j + start_y];

	/* Shrink matrix */
	for (size_t i = 0; i < img->height; ++i) {
		if (i < new_height) {
			img->pixels[i] =
				realloc(img->pixels[i],
					new_width * sizeof(*img->pixels[i]));
			assert(img->pixels[i] != NULL);
		} else {
			free(img->pixels[i]);
		}
	}
	img->pixels = realloc(img->pixels, img->height * sizeof(*img->pixels));
	assert(img->pixels != NULL);

	img->width = end_y - start_y + 1;
	img->height = end_x - start_x + 1;
	return 0;
}

static int resize_image(struct image *img, size_t new_width, size_t new_height)
{
	/* Free rows which will be lost */
	if (new_height < img->height)
		for (size_t i = new_height; i < img->height; ++i)
			free(img->pixels[i]);

	/* Resize height */
	img->pixels = realloc(img->pixels, new_height * sizeof(*img->pixels));
	assert(img->pixels != NULL);

	/* Make sure realloc does not fail, below */
	if (new_height > img->height)
		memset(img->pixels + img->height, 0,
		       (new_height - img->height) * sizeof(*img->pixels));

	/* Resize width */
	for (size_t i = 0; i < new_height; ++i) {
		img->pixels[i] = realloc(img->pixels[i],
					 new_width * sizeof(*img->pixels[i]));
		assert(img->pixels[i] != NULL);
	}

	/* Fill with white pixels */
	for (size_t i = 0; i < new_height; ++i)
		for (size_t j = 0; j < new_width; ++j)
			if (i >= img->height || j >= img->width) {
				img->pixels[i][j].r = 255;
				img->pixels[i][j].g = 255;
				img->pixels[i][j].b = 255;
			}

	img->width = new_width;
	img->height = new_height;
	return 0;
}

static int color_image(struct image *img, size_t start_x, size_t end_x,
		       size_t start_y, size_t end_y, struct pixel pix)
{
	for (size_t i = start_x; i <= end_x; ++i)
		for (size_t j = start_y; j <= end_y; ++j)
			img->pixels[i][j] = pix;

	return 0;
}

static int blur_image(struct image *img, int iters)
{
	for (int i = 0; i < iters; ++i) {
		struct image img_copy;
		copy_image(img, &img_copy);

		for (size_t i = 0; i < img->height; ++i)
			for (size_t j = 0; j < img->width; ++j) {
				unsigned r_sum = 0, g_sum = 0, b_sum = 0;
				unsigned neighs = 0;

				if (i > 0) {
					r_sum += img->pixels[i - 1][j].r;
					g_sum += img->pixels[i - 1][j].g;
					b_sum += img->pixels[i - 1][j].b;
					++neighs;
				}
				if (i < img->height - 1) {
					r_sum += img->pixels[i + 1][j].r;
					g_sum += img->pixels[i + 1][j].g;
					b_sum += img->pixels[i + 1][j].b;
					++neighs;
				}
				if (j > 0) {
					r_sum += img->pixels[i][j - 1].r;
					g_sum += img->pixels[i][j - 1].g;
					b_sum += img->pixels[i][j - 1].b;
					++neighs;
				}
				if (j < img->width - 1) {
					r_sum += img->pixels[i][j + 1].r;
					g_sum += img->pixels[i][j + 1].g;
					b_sum += img->pixels[i][j + 1].b;
					++neighs;
				}

				img_copy.pixels[i][j].r = r_sum / neighs;
				img_copy.pixels[i][j].g = g_sum / neighs;
				img_copy.pixels[i][j].b = b_sum / neighs;
			}

		SWAP(img, &img_copy);
		free_image(&img_copy, 1);
	}

	return 0;
}

static void flip_180(struct image *img)
{
	for (size_t i = 0; i < img->height; ++i)
		for (size_t lhs = 0, rhs = img->width - 1; lhs < rhs;
		     ++lhs, -- rhs)
			SWAP(&img->pixels[i][lhs], &img->pixels[i][rhs]);

	for (size_t lhs = 0, rhs = img->height - 1; lhs < rhs; ++lhs, --rhs)
		SWAP(&img->pixels[lhs], &img->pixels[rhs]);
}

static void flip_90(struct image *img)
{
	struct image new_img;
	new_image(&new_img, img->height, img->width);

	for (size_t i = 0; i < new_img.height; ++i)
		for (size_t j = 0; j < new_img.width; ++j)
			new_img.pixels[i][j] =
				img->pixels[new_img.width - j - 1][i];

	SWAP(img, &new_img);
	free_image(&new_img, 1);
}

static int rotate_image(struct image *img, int count)
{
	switch (count) {
	case 1:
		flip_90(img);
		break;
	case 2:
		flip_180(img);
		break;
	case 3:
		flip_90(img);
		flip_180(img);
		break;
	}
	return 0;
}

static int pixels_equal(struct pixel *lhs, struct pixel *rhs)
{
	return lhs->r == rhs->r && lhs->g == rhs->g && lhs->b == rhs->b;
}

static int fill_image(struct image *img, size_t i, size_t j, struct pixel pix)
{
	struct pixel ref_pix = img->pixels[i][j];

	if (pixels_equal(&pix, &ref_pix))
		return 0;

	img->pixels[i][j] = pix;

	if (i > 0 && pixels_equal(&img->pixels[i - 1][j], &ref_pix))
		fill_image(img, i - 1, j, pix);

	if (i < img->height - 1 &&
	    pixels_equal(&img->pixels[i + 1][j], &ref_pix))
		fill_image(img, i + 1, j, pix);

	if (j > 0 && pixels_equal(&img->pixels[i][j - 1], &ref_pix))
		fill_image(img, i, j - 1, pix);

	if (j < img->width - 1 &&
	    pixels_equal(&img->pixels[i][j + 1], &ref_pix))
		fill_image(img, i, j + 1, pix);

	return 0;
}

static void print_image(struct image *img)
{
	printf("%zu %zu\n", img->width, img->height);
	for (size_t i = 0; i < img->height; ++i) {
		for (size_t j = 0; j < img->width; ++j)
			printf("%hhu %hhu %hhu ", img->pixels[i][j].r,
			       img->pixels[i][j].g, img->pixels[i][j].b);
		printf("\n");
	}
	fflush(stdout);
}

int main(void)
{
	int op;
	struct image img;
	unsigned char init = 0;

	while (scanf("%d", &op) == 1 && op != 0) {

		size_t width, height;
		size_t start_x, end_x, start_y, end_y;
		struct pixel pix;
		int iters;

        if (op != 1) {
            if (!init) {
                fprintf(stderr, "%d\n", EPERM);
                exit(EXIT_FAILURE);
            }
        }

		switch (op) {
		case OP_INIT:
			scanf("%zu%zu", &width, &height);

            free_image(&img, init);

			if (!check_arg_bounds(height, 1, HEIGHT_MAX + 1) ||
			    !check_arg_bounds(width, 1, WIDTH_MAX + 1)) {
			    exit(EXIT_FAILURE);
			}

			init_image(&img, width, height);
			init = 1;
			break;

		case OP_CROP:
			scanf("%zu%zu%zu%zu", &start_y, &start_x, &end_y, &end_x);

		    if (!check_arg_bounds(start_x, 0, img.height) ||
		        !check_arg_bounds(end_x, 0, img.height)   ||
		        !check_arg_bounds(start_y, 0, img.width)  ||
		        !check_arg_bounds(end_y, 0, img.width)) {
                free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

		    if (start_y > end_y || start_x > end_x) {
		        fprintf(stderr, "%d\n", EINVAL);
		        free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

			crop_image(&img, start_x, end_x, start_y, end_y);
			break;

		case OP_RESIZE:
			scanf("%zu%zu", &width, &height);

			if (!check_arg_bounds(height, 1, HEIGHT_MAX + 1) ||
			    !check_arg_bounds(width, 1, WIDTH_MAX + 1)) {
                free_image(&img, init);
			    exit(EXIT_FAILURE);
			}

			resize_image(&img, width, height);
			break;

		case OP_COLOR:
			scanf("%zu%zu%zu%zu", &start_y, &start_x, &end_y, &end_x);
			read_pixel(&pix);

			if (!check_arg_bounds(start_x, 0, img.height) ||
		        !check_arg_bounds(end_x, 0, img.height)) {
                free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

		    if (!check_arg_bounds(start_y, 0, img.width) ||
		        !check_arg_bounds(end_y, 0, img.width)) {
                free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

		    if (start_y > end_y || start_x > end_x) {
		        fprintf(stderr, "%d\n", EINVAL);
		        free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

			color_image(&img, start_x, end_x, start_y, end_y, pix);
			break;

		case OP_BLUR:
			scanf("%d", &iters);

			if (iters < 0) {
			    fprintf(stderr, "%d\n", EINVAL);
                free_image(&img, init);
			    exit(EXIT_FAILURE);
			}

			blur_image(&img, iters);
			break;

		case OP_ROTATE:
			scanf("%d", &iters);

            if (!check_arg_bounds(iters, 1, 4)) {
                free_image(&img, init);
                exit(EXIT_FAILURE);
            }

			rotate_image(&img, iters);
			break;

		case OP_FILL:
			scanf("%zu%zu", &start_y, &start_x);

			if (!check_arg_bounds(start_x, 0, img.height) ||
		        !check_arg_bounds(start_y, 0, img.width)) {
                free_image(&img, init);
		        exit(EXIT_FAILURE);
		    }

			read_pixel(&pix);
			fill_image(&img, start_x, start_y, pix);
			break;

		case OP_PRINT:

			print_image(&img);
			break;

		default:
		    fprintf(stderr, "%d\n", EPERM);
        	free_image(&img, init);
		    exit(EXIT_FAILURE);
		}
	}

	free_image(&img, init);
	return 0;
}
