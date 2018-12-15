//1. Create Game Window .. OK
//2. Draw Mandelbrot .. OK
//3. Click and Zoom ..OK
//4. Move by cursor
//5. Coloring
//6. openMP

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

const int W = 1920;
const int H = 1080;
int max_iter = 32;
double zoom = 1.0;
double minr = -2.5, maxr = 1;
double mini = -1, maxi = 1;

// computes v + t(u - v)
// t should be a value between 0 and 1
Color linear_interpolation(const Color& v, const Color& u, double a)
{
	auto const b = 1 - a;
	return Color(b*v.r + a * u.r,
		b*v.g + a * u.g,
		b*v.b + a * u.b);
}

int main(void)
{
	RenderWindow window(VideoMode(W, H), "mandelbrot");
	Image        image;image.create(W, H);
	Texture t;
	Sprite s;

	Font font; 
	bool r = font.loadFromFile("arial.ttf");
	assert(r);
	Text text;
	text.setFont(font);
	text.setCharacterSize(24);
	text.setFillColor(Color::White);
	

	while (window.isOpen())
	{
		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed) window.close();
			if (e.type == Event::KeyPressed)
			{
				double w = (maxr - minr) * 0.3;
				double h = (maxi - mini) * 0.3;
				if (e.key.code == Keyboard::Left)
				{
					minr -= w;
					maxr -= w;
				}
				if (e.key.code == Keyboard::Right)
				{
					minr += w, maxr += w;
				}
				if (e.key.code == Keyboard::Up)
				{
					mini -= h, maxi -= h;
				}
				if (e.key.code == Keyboard::Down)
				{
					mini += h, maxi += h;
				}
			}
			if (e.type == Event::MouseButtonPressed)
			{
				auto set_zoom = [&](double z)
				{
					//mouse point will be new center point
					double cr = minr + 1.0*(maxr - minr)*e.mouseButton.x / W;
					double ci = mini + 1.0*(maxi - mini)*e.mouseButton.y / H;

					//zoom
					double tminr = cr - (maxr - minr) / 2 / z;
					maxr = cr + (maxr - minr) / 2 / z;
					minr = tminr;

					double tmini = ci - (maxi - mini) / 2 / z;
					maxi = ci + (maxi - mini) / 2 / z;
					mini = tmini;
				};

				if (e.mouseButton.button == Mouse::Left)
				{
					zoom *= 5;
					set_zoom(5);
				}
				if (e.mouseButton.button == Mouse::Right)
				{
					zoom /= 5;
					set_zoom(0.2);
				}
			}
			if (e.type == Event::MouseWheelScrolled)
			{
				if (e.mouseWheelScroll.wheel == Mouse::VerticalWheel)
				{
					if (e.mouseWheelScroll.delta > 0) max_iter *= 2;
					else max_iter /= 2;
					if (max_iter < 1)max_iter = 1;
				}

			}
		}
		window.clear();
#pragma omp parallel for
		for (int y = 0; y < H; y++)
		{
			for (int x = 0; x < W; x++)
			{
				double cr = minr + 1.0*(maxr - minr)*x / W;
				double ci = mini + 1.0*(maxi - mini)*y / H;
				double r = 0, i = 0;
				if (x == W / 2 && y == H / 2)
				{
					int sdf = 1;
				}
				int iter;
				for (iter = 0; iter < max_iter; iter++)
				{
					double tr = r * r - i * i + cr;
					i = 2 * r*i + ci;
					r = tr;
					if (r*r + i * i > 2 * 2) break;
				}

				int rr = 1.0*(max_iter - iter) / max_iter * 0xff;
				int gg = rr, bb = rr;
				int color = (rr << 16) | (gg << 8) | (bb << 0);

				/*
				static const std::vector<Color> colors{
					{  0,   0,   0},
					{213,  67,  31},
					{251, 255, 121},
					{ 62, 223,  89},
					{ 43,  30, 218},
					{  0, 255, 247}
				};
				*/
				//From Ultra Fractal and Wikipedia
				static const std::vector<Color> colors{
					{  0,   7,   100},
					{32, 107, 203},
					{237, 255, 255},
					{ 255, 170, 0},
					{0, 2, 0},
				};

				static const auto max_color = colors.size() - 1;
				if (iter == max_iter) iter = 0;
				double mu = 1.0* iter / max_iter;
				// scale mu to be in the range of colors
				mu *= max_color;
				auto i_mu = static_cast<std::size_t>(mu);
				auto color1 = colors[i_mu];
				auto color2 = colors[std::min(i_mu + 1, max_color)];
				Color c = linear_interpolation(color1, color2, mu - i_mu);

				image.setPixel(x, y, c);

			}
		}

		t.loadFromImage(image);
		s.setTexture(t);
		window.draw(s);

		char str[100];
		sprintf(str, "max iter:%d\nzoom:x%2.2lf", max_iter, zoom);
		text.setString(str);

		window.draw(text);



		window.display();
	}

	return 0;
}