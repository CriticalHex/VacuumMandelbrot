#include<iostream>
#include<math.h>
#include<SFML\Graphics.hpp>
#include<complex>
#include<thread>
#include<vector>
#include<list>

using namespace std;

class Pixel { //Class for each pixel in the pixel vector.
public:		  //Stores all the information needed per pixel.
	Pixel(complex<double> loc, long double pos[2]) { //constructor
		c = loc; //c part of the mandelbrot set
		position[0] = pos[0]; //the pixel coordinate of that c value
		position[1] = pos[1];
	}
	bool iterate(sf::Image& image) { //the function that does the mandelbrot calculations.
		if (!escaped) { //don't calculate if it already escaped.
			z = (pow(z, 2)) + c;
			count += 1;
			if (abs(z) >= 2) { //if it's greater than 2 it's guaranteed to have escaped.
				color = sf::Color((count * 8), (count * 6), (count * 12)); //only after it escapes do we set a color.
				escaped = true;
			}
		}
		image.setPixel(position[0], position[1], color); //draw the pixel, either white(not escaped) or any other color(escaped)
		return escaped; //unneccesary currently, I was looking to use this to trim the array length for faster computing but I didn't have any luck
	}
private:
	long double position[2]; //pixel coord
	complex<double> c; //c part
	complex<double> z = (0, 0); //starting value
	int count = 0; //how many times did it iterate before escaping
	bool escaped = false; //read
	sf::Color color = sf::Color(255,255,255); //color value, default White
};

void fill_array(vector<Pixel*>& pixels, long double scale, int width, int height, sf::Vector2f mouse) { //fills the pixel array with Pixel objects
	long double shift[2] = { 0, 0 };
	long double horizontalStart;
	long double horizontalEnd;
	long double verticalStart;
	long double verticalEnd;
	long double origin[2] = { (width / 2), (height / 2) };
	long double scaleStart = (-2 / scale); //the actual "zooming in" of the math part
	long double scaleEnd = (2 / scale);
	long double horizontalSize = (width / (abs(scaleStart) + abs(scaleEnd))); //for scaling the c value up to pixel coords, 
	long double verticalSize = (height / (abs(scaleStart) + abs(scaleEnd))); //gets bigger as we zoom in
	long double horizontalRes = (abs(scaleStart) + abs(scaleEnd)) / width * 1; // step between c values.
	long double verticalRes = (abs(scaleStart) + abs(scaleEnd)) / height * 1; // see below 

	/*the screen resolution is 1000, so we need 1000 steps between the start and end of the "graph", our screen.
	starting at the default scale, our graph, so the c values, range from -2 - 2, which is a total range of 4,
	hence the absolute value taken above. we need 1000 steps, for the 1000 pixels, so the resolution is 4 / 1000.*/

	shift[0] = origin[0] + ((origin[0] - mouse.x) * scale); //this is the zoom on the mouse part. i only regret not making it scale,
	shift[1] = origin[1] + ((origin[1] - mouse.y) * scale);//as in, zoom to where your mouse is on the zoomed image.

	horizontalStart = scaleStart + ((origin[0] - shift[0]) / horizontalSize); //this is the actual calculation of the start and end ranges, -2 - 2 in our previous example,
	horizontalEnd = scaleEnd + ((origin[0] - shift[0]) / horizontalSize);    //but shifted to center around the mouse. 
	verticalStart = scaleStart + ((origin[1] - shift[1]) / verticalSize);
	verticalEnd = scaleEnd + ((origin[1] - shift[1]) / verticalSize);

	long double position[2] = { 0, 0 };
	for (long double t = horizontalStart; t < horizontalEnd; t += horizontalRes) { //loop through the cols

		for (long double m = verticalStart; m < verticalEnd; m += verticalRes) { //loop through the rows

			position[0] = int((t * horizontalSize) + shift[0]); //i know this is ints, and the data type is long double.
			position[1] = int((m * verticalSize) + shift[1]);  //i'll be messing with ints and doubles more so if this isn't true anymore, oh well.

			if (position[0] >= 0 and position[0] <= width and position[1] >= 0 and position[1] <= height) { //are we on the screen? if not, why would we calculate those pixels?

				complex<double> c(t, m); //the graph is technically on the complex plane, this is just where we implement that.

				pixels.push_back(new Pixel(c, position)); //creation of the pixel object.
			}
		}
	}
}

void draw(vector<Pixel*>& pixels, sf::Image& image, int index_begin, int index_end) { //called by the threads to call the iterate function on every pixel.
		for (int i = index_begin; i < index_end; i++) { //the thread division
			pixels[i]->iterate(ref(image)); //call iterate with a reference to the sf::Image
		}
}

vector<thread> create_threads(vector<Pixel*>& pixels, sf::Image& image, int max) { //creates the threads for multithreading
	vector<thread> threads; //initialize vector to fill
	int section = pixels.size() / max;;
	for (int i = 0; i < max; i++) { // for max threads:
		threads.emplace_back(draw, ref(pixels), ref(image), section * i, (section * (i + 1)) - 1); //divides the screen evenly amongst threads, threads call the draw function.
	}
	return threads; //give the thread vector to main
}


int main() {
	//RENDER SETUP----------------------------------------------------------
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Mandelbrot Set", sf::Style::None); //window without title bar
	window.setPosition(sf::Vector2i(460, 20)); //put window where i want it. yes these are magic numbers.

	//VARIABLES-------------------------------------------------------------
	sf::Color bgColor = sf::Color(8, 6, 12); //the color that one iteration before escape returns.
	int zooms = 0; //max 43 before old error shows up, hopefully a future fix
	long double scale = pow(2, zooms); //level of zoom in
	
	sf::Event event;
	vector<thread> active_threads;
	int max_threads = 12;
	sf::Image image; //image to change pixels in, in the iterate function.
	image.create(window.getSize().x, window.getSize().y, bgColor);
	sf::Texture texture; //these three lines are just to be able to draw the image :(
	texture.loadFromImage(image); //
	sf::Sprite mandel(texture); //
	vector<Pixel*> pixels;
	vector<Pixel*>::iterator iter;
	sf::CircleShape dot(1); //these three are the dot in the center of the screen so you can tell where you're zooming.
	dot.setFillColor(sf::Color::Green);
	dot.setPosition(500, 500);

	//INITIAL SET CREATION---------------------------------------------------
	fill_array(ref(pixels), scale, window.getSize().x, window.getSize().y, sf::Vector2f(500, 500));

	//GAME LOOP--------------------------------------------------------------
	while (window.isOpen()) {
		while (window.pollEvent(event))
		{
			// Close window: exit--------------------------------------------------------------------------------------------------------------------------
			if (event.type == sf::Event::Closed)
				window.close();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { //press lcontrol: exit
				window.close();
			}

			if (event.type == sf::Event::MouseWheelScrolled) {
				if (event.mouseWheelScroll.delta > 0) { //zoom in
					scale *= 2;
					zooms += 1;
				}
				else if (event.mouseWheelScroll.delta < 0) { //zoom out, why would you ever want to?
					scale /= 2;
					zooms -= 1;
				}
				pixels.clear(); //reset the pixels.
				fill_array(ref(pixels), scale, window.getSize().x, window.getSize().y, sf::Vector2f(62.5003972516, 500));
				cout << zooms << endl;
				//cout << event.mouseWheelScroll.x << ", " << event.mouseWheelScroll.y << endl;
				//60.5379, 495.55110165
				//event.mouseWheelScroll.x, event.mouseWheelScroll.y
				//567.844982197, 498.716377632
				//590.060110859, 339.671734734 from video
				//62.5003972516, 500 other video
			}
		}
		//RENDER--------------------------------------------------------------
		window.clear(bgColor);

		active_threads = create_threads(ref(pixels), ref(image), max_threads); //create threads to iterate drawing.
		for (auto& th : active_threads) {
			if (th.joinable()) {
				th.join(); //wait for threads to exit before continuing. without this it crashes.
			}
		}
		active_threads.clear(); //just to be sure

		texture.loadFromImage(image); //update the sprite with the image
		window.draw(mandel); //draw the sprite

		window.draw(dot); //center dot
		window.display(); //update screen
	}
}