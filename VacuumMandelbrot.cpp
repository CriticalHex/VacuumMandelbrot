#include<iostream>
#include<math.h>
#include<SFML\Graphics.hpp>
#include<complex>
#include<thread>
#include<vector>
#include<list>

using namespace std;

class Pixel {
public:
	Pixel(complex<double> c, sf::Vector2f position) {
		complex<double> c = c;
		sf::Vector2f position = position;
	}
	void iterate() {
		if (!escaped) {
			z = (pow(z, 2)) + c;
			count += 1;
			if (abs(z) >= 2) {
				color = sf::Color((count * 8) % 256, (count * 6) % 256, (count * 12) % 256);
				escaped = true;
			}
		}
	}
	void draw(sf::Image& image) {
		image.setPixel(position.x, position.y, color);
	}
private:
	sf::Vector2f position;
	complex<double> c;
	complex<double> z = (0, 0);
	int count = 0;
	bool escaped = false;
	sf::Color color = sf::Color(255,255,255);
};

void fill_array(vector<Pixel*>& pixels, float scale, int width, int height, sf::Vector2f mouse) {
	sf::Vector2f shift;
	long double horizontalStart;
	long double horizontalEnd;
	long double verticalStart;
	long double verticalEnd;
	sf::Vector2f origin((width / 2), (height / 2));
	long double scaleStart = (-2 / scale);
	long double scaleEnd = (2 / scale);
	long double horizontalSize = (width / (abs(scaleStart) + abs(scaleEnd)));
	long double verticalSize = (height / (abs(scaleStart) + abs(scaleEnd)));
	long double horizontalRes = (abs(scaleStart) + abs(scaleEnd)) / width * 1;
	long double verticalRes = (abs(scaleStart) + abs(scaleEnd)) / height * 1;

	shift = origin + ((origin - mouse) * scale);

	horizontalStart = scaleStart + ((origin.x - shift.x) / horizontalSize);
	horizontalEnd = scaleEnd + ((origin.x - shift.x) / horizontalSize);
	verticalStart = scaleStart + ((origin.y - shift.y) / verticalSize);
	verticalEnd = scaleEnd + ((origin.y - shift.y) / verticalSize);

	sf::Vector2f position;
	int num;
	for (long double t = horizontalStart; t < horizontalEnd; t += horizontalRes) {

		for (long double m = verticalStart; m < verticalEnd; m += verticalRes) {

			position.x = int((t * horizontalSize) + shift.x);
			position.y = int((m * verticalSize) + shift.y);

			if (position.x >= 0 and position.x <= width and position.y >= 0 and position.y <= height) {

				complex<double> c(t, m);

				pixels.push_back(new Pixel(c, position));
			}
		}
	}
}

int main() {
	//RENDER SETUP----------------------------------------------------------
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Mandelbrot Set", sf::Style::None);
	window.setPosition(sf::Vector2i(460, 20));
	window.setFramerateLimit(60);

	//VARIABLES-------------------------------------------------------------
	sf::Color bgColor = sf::Color(8, 6, 12);
	float scale = 1;
	scale = pow(2, 25);
	sf::Event event;
	/*list<thread> active_threads;
	int max_threads = 16;*/
	sf::Image image;
	image.create(window.getSize().x, window.getSize().y, bgColor);
	sf::Texture tex;
	tex.loadFromImage(image);
	sf::Sprite mandel(tex);
	vector<Pixel*> pixels;
	sf::CircleShape dot(1);
	dot.setFillColor(sf::Color::Green);
	dot.setPosition(500, 500);

	//INITIAL SET CREATION---------------------------------------------------
	fill_array(pixels, scale, window.getSize().x, window.getSize().y, sf::Vector2f(500, 500));

	//GAME LOOP--------------------------------------------------------------
	while (window.isOpen()) {
		while (window.pollEvent(event))
		{
			// Close window: exit--------------------------------------------------------------------------------------------------------------------------
			if (event.type == sf::Event::Closed)
				window.close();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
				window.close();
			}

			if (event.type == sf::Event::MouseWheelScrolled) {
				if (event.mouseWheelScroll.delta > 0) {
					scale *= 2;
				}
				else if (event.mouseWheelScroll.delta < 0) {
					scale /= 2;
				}
				fill_array(pixels, scale, window.getSize().x, window.getSize().y, sf::Vector2f(event.mouseWheelScroll.x, event.mouseWheelScroll.y));
				cout << scale << endl;
				//cout << event.mouseWheelScroll.x << ", " << event.mouseWheelScroll.y << endl;
				//60.538 - .0001, 495.551 + .0001
				//event.mouseWheelScroll.x, event.mouseWheelScroll.y
				//567.844982197, 498.716377632
			}
		}
		//RENDER--------------------------------------------------------------
		window.clear(bgColor);
		for (auto& iter : pixels) {
			iter->iterate();
			iter->draw(image);
		}
		window.draw(mandel);
		window.draw(dot);
		window.display();
	}
}