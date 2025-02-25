description("Tests for the imageSmoothingEnabled attribute.");
var ctx = document.createElement('canvas').getContext('2d');

debug("Test that the default value is true.");
shouldBe("ctx.webkitImageSmoothingEnabled", "true");

debug("Test that false is returned after a the attribute is set to false.");
ctx.webkitImageSmoothingEnabled = false;
shouldBe("ctx.webkitImageSmoothingEnabled", "false");

var image = document.createElement('canvas');
image.width = 2;
image.height = 1;

// We use this to colour the individual pixels
var dotter = ctx.createImageData(1, 1);

// Colour the left pixel black.
dotter.data[0] = 0;
dotter.data[1] = 0;
dotter.data[2] = 0;
dotter.data[3] = 255;
image.getContext('2d').putImageData(dotter, 0, 0);

// Colour the right pixel white.
dotter.data[0] = 255;
dotter.data[1] = 255;
dotter.data[2] = 255;
dotter.data[3] = 255;
image.getContext('2d').putImageData(dotter, 1, 0);


debug("New canvas element created.");
var canvas = document.createElement('canvas');
canvas.width = 4;
canvas.height = 1;
ctx = canvas.getContext('2d');
ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
left_of_center_pixel = ctx.getImageData(1, 0, 1, 1);

debug("Test that the image is smoothed by default. We check by making sure the pixel just to the left of the middle line is not completely black.");
shouldBe("left_of_center_pixel.data[0] !== 0", "true");
shouldBe("left_of_center_pixel.data[1] !== 0", "true");
shouldBe("left_of_center_pixel.data[2] !== 0", "true");

ctx.webkitImageSmoothingEnabled = false;
ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
left_of_center_pixel = ctx.getImageData(1, 0, 1, 1);
debug("Test that nearest neighbour is used when imageSmoothingEnabled is set to false. We check this by making sure the pixel just to the left of the middle line is completely black.");
shouldBe("left_of_center_pixel.data[0]", "0");
shouldBe("left_of_center_pixel.data[1]", "0");
shouldBe("left_of_center_pixel.data[2]", "0");

ctx.webkitImageSmoothingEnabled = true;
ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
left_of_center_pixel = ctx.getImageData(1, 0, 1, 1);
debug("Test that the image is smoothed when imageSmoothingEnabled is set to true.");
shouldBe("left_of_center_pixel.data[0] !== 0", "true");
shouldBe("left_of_center_pixel.data[1] !== 0", "true");
shouldBe("left_of_center_pixel.data[2] !== 0", "true");

