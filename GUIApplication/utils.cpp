#include "utils.h"

void get3Points(int n,int&x,int&y,int&z){
    x=rand()%n;
    do{
        y=rand()%n;
    }
    while (y==x);

    do{
        z=rand()%n;
    }
    while(z==x ||z==y);
}

double dist2plane(QVector3D point, QVector3D plane){
    double num = abs(plane.x()*point.x() + plane.y()*point.y() + plane.z()*point.z() - 1);
    double den = sqrt(plane.x()*plane.x() + plane.y()*plane.y() + plane.z()*plane.z());
    return num/den;
}

void fit_plane(QVector<QVector3D> points, QVector3D& maybe_model, double& error){
    int num_points = points.size();

    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixCustom;
    MatrixCustom A(num_points, 3), B(num_points, 1);

    for (int i = 0; i < num_points; ++i){
        A(i,0) = points[i].x();
        A(i,1) = points[i].y();
        A(i,2) = points[i].z();
        B(i,0) = 1;
    }
    MatrixCustom X = (A.transpose() * A).inverse() * A.transpose() * B;
    maybe_model = QVector3D(X(0,0), X(1,0), X(2,0));

    MatrixCustom E = B - A * X;
    error = 0;
    for (int i=0; i<num_points; i++){
        error += E(i,0) * E(i,0);
    }
}

bool close_enough(QVector3D point, QVector3D plane, double error_limit){
    return dist2plane(point, plane) < error_limit;
}

bool good_enough(QVector<QVector3D> points, int good_num_points){
    return points.size() > good_num_points;
}

bool between_corners(QMatrix4x4 transform, QVector3D point, QVector3D c1, QVector3D c2){
    QVector4D tp = transform * QVector4D(point, 1.0f);
    QVector3D point3D = QVector3D(tp.x()/tp.z(), tp.y()/tp.z(), tp.y()/tp.z());
    c1 = QVector2D(((c1.x()-START_X) - SIZE_X/2.0)/(SIZE_X/2.0), -((c1.y()-START_Y) - SIZE_Y/2.0)/(SIZE_Y/2.0));
    c2 = QVector2D(((c2.x()-START_X) - SIZE_X/2.0)/(SIZE_X/2.0), -((c2.y()-START_Y) - SIZE_Y/2.0)/(SIZE_Y/2.0));

    if(c1.x() <= c2.x() and c1.y() <= c2.y()){ // c1 TL : c2 BR
        return (c1.x() <= point3D.x() and
                point3D.x() <= c2.x() and
                c1.y() <= point3D.y() and
                point3D.y() <= c2.y());

    } else if(c1.x() <= c2.x() and c1.y() >= c2.y()){ // c1 BL : c2 TR
        return (c1.x() <= point3D.x() and
                point3D.x() <= c2.x() and
                c2.y() <= point3D.y() and
                point3D.y() <= c1.y());

    } else if(c1.x() >= c2.x() and c1.y() <= c2.y()){ // c1 TR : c2 BL
        return (c2.x() <= point3D.x() and
                point3D.x() <= c1.x() and
                c1.y() <= point3D.y() and
                point3D.y() <= c2.y());

    } else { // c1 BR : c2 TL
        return (c2.x() <= point3D.x() and
                point3D.x() <= c1.x() and
                c2.y() <= point3D.y() and
                point3D.y() <= c1.y());
    }
}

QVector3D pointOnPlane(QVector3D point, QVector3D plane){
    float t = (1 - point.x() + point.y() + point.z())/plane.lengthSquared();
    return point + t*plane;
}

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <png.h>

GLuint png_texture_load(const char * file_name)
{
    // This function was originally written by David Grayson for
    // https://github.com/DavidEGrayson/ahrs-visualizer

    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        return 0;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

//    printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

    if (bit_depth != 8)
    {
        fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
        return 0;
    }

    GLint format, internal_format;
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
         internal_format = format = GL_RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        internal_format = format = GL_RGBA;
        break;
    case PNG_COLOR_TYPE_GRAY:
        internal_format = GL_RED;
        format = GL_RED;
        break;
    default:
        fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
        return 0;
    }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    return texture;
}

GLuint distorted_texture_load(cv::Mat img){
    GLuint textureTrash;
    cv::flip(img, img, 0);
    glGenTextures(1, &textureTrash);
    glBindTexture(GL_TEXTURE_2D, textureTrash);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RED,            // Internal colour format to convert to
                 img.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 img.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 GL_RED, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 img.ptr());        // The actual image data itself

    glGenerateMipmap(GL_TEXTURE_2D);
    return textureTrash;
}
