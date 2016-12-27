#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;


// Code from: http://www.adp-gmbh.ch/cpp/common/base64.html

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string base64_encode(uchar const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i <4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

string mat2str(const Mat& m)
{
    Mat src;
    if (!m.isContinuous()) {
        src = m.clone();
    }
    else {
        src = m;
    }

    // Create header
    int type = m.type();
    int channels = m.channels();
    vector<uchar> data(4*sizeof(int));
    memcpy(&data[0 * sizeof(int)], (uchar*)&m.rows, sizeof(int));
    memcpy(&data[1 * sizeof(int)], (uchar*)&m.cols, sizeof(int));
    memcpy(&data[2 * sizeof(int)], (uchar*)&type, sizeof(int));
    memcpy(&data[3 * sizeof(int)], (uchar*)&channels, sizeof(int));

    // Add image data
    data.insert(data.end(), m.datastart, m.dataend);

    // Encode
    return base64_encode(data.data(), data.size());
}

Mat str2mat(const string& s)
{
    // Decode data
    string data = base64_decode(s);

    // Decode Header
    int rows;
    int cols;
    int type;
    int channels;
    memcpy((char*)&rows, &data[0 * sizeof(int)], sizeof(int));
    memcpy((char*)&cols, &data[1 * sizeof(int)], sizeof(int));
    memcpy((char*)&type, &data[2 * sizeof(int)], sizeof(int));
    memcpy((char*)&channels, &data[3 * sizeof(int)], sizeof(int));

    // Make the mat
    return Mat(rows, cols, type, (uchar*)&data[4*sizeof(int)]).clone();
}

int main()
{
	std::string encoded;
	
    {
        Mat3b m(100, 100, Vec3b(0, 0));
        circle(m, Point(50, 50), 25, Scalar(0, 255, 0));

		Mat image = imread("download.jpg", CV_LOAD_IMAGE_COLOR);
		imshow("download", image);
        imshow("Original", m);
        waitKey(1);

        encoded = mat2str(image);
		const char *_encoded = encoded.c_str();
    }
	printf("encoded = %s\n", _encoded);
    Mat decoded = str2mat(encoded);
    imshow("Reconstructed", decoded);
    waitKey();

    return 0;
}
