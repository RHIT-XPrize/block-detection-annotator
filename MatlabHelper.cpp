//-----------------------------------------------------------------------------
// <copyright file="MatlabHelper.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------------

#include "MatlabHelper.h"

/// <summary>
/// Constructor
/// </summary>
MatlabHelper::MatlabHelper() :
    m_depthFilterID(NO_FILTER),
    m_colorFilterID(NO_FILTER),
    m_matlabEngine(NULL)
{
}

/// <summary>
/// Destructor
/// </summary>
MatlabHelper::~MatlabHelper() 
{
}

void MatlabHelper::ShutDownEngine()
{
    if (m_matlabEngine)
    {
        // Shutdown MATLAB engine session
        engClose(m_matlabEngine);
    }
}

/// <summary>
/// Starts a MATLAB engine session
/// </summary>
/// <param name="engineUIVisible">whether to show the MATLAB engine GUI</param>
/// <returns>S_OK if successful, an error code otherwise
HRESULT MatlabHelper::InitMatlabEngine(bool engineUIVisible /* = false */)
{
    // Start a MATLAB engine session and get a handle back
    m_matlabEngine = engOpen(NULL);
    if (!m_matlabEngine)
    {
        return E_NOT_VALID_STATE;
    }

    // Show/hide the MATLAB engine UI
    int result = engSetVisible(m_matlabEngine, engineUIVisible);
    if (result != 0)
    {
        return E_NOT_VALID_STATE;
    }

    return S_OK;
}


/// <summary>
/// Sets the color image filter to the one corresponding to the given resource ID
/// </summary>
/// <param name="filterID">resource ID of filter to use</param>
void MatlabHelper::SetColorFilter(int filterID)
{
    m_colorFilterID = filterID;
}

/// <summary>
/// Sets the depth image filter to the one corresponding to the given resource ID
/// </summary>
/// <param name="filterID">resource ID of filter to use</param>
void MatlabHelper::SetDepthFilter(int filterID)
{
    m_depthFilterID = filterID;
}

/// <summary>
/// Applies the color image filter to the given image
/// </summary>
/// <param name="pImg">pointer to mxArray holding image to filter</param>
/// <returns>S_OK if successful, an error code otherwise
HRESULT MatlabHelper::ApplyColorFilter(mxArray* pImg)
{
    // Check to see if we have a valid engine pointer
    if (!m_matlabEngine) 
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
    }

    // Validate RGB matrix
    HRESULT hr = ValidateMxArrayRgbImage(pImg);
    if (FAILED(hr))
    {
        return hr;
    }

    mxArray* filteredImage = NULL;
	std::vector<std::vector<double> > finalCentroids;

    // Apply an effect based on the active filter
	if (m_colorFilterID == IDM_COLOR_GRAYSCALE_THRESHOLD) {
		hr = ComputeCentroids(pImg, finalCentroids);
	}

    return hr;
}


/// <summary>
/// Converts an RGB MATLAB mxArray into a Windows GDI bitmap
/// </summary>
/// <param name="pImg">pointer to mxArray holding image to convert</param>
/// <param name="ppBitmapBits">pointer to pointer that will point to converted bitmap data</param>
/// <param name="pBitmapInfo">header info for converted bitmap data</param>
/// <returns>S_OK if successful, an error code otherwise</returns>
HRESULT MatlabHelper::ConvertRgbMxArrayToBitmap(const mxArray* pImg, void** bitmapBits, BITMAPINFO* pBitmapInfo)
{
    if (!bitmapBits)
    {
        return E_POINTER;
    }

    // Validate RGB matrix
    HRESULT hr = ValidateMxArrayRgbImage(pImg);
    if (FAILED(hr))
    {
        return hr;
    }

    const mwSize* dimensions = mxGetDimensions(pImg);
    const int height = static_cast<int>(dimensions[0]);
    const int width = static_cast<int>(dimensions[1]);

    // Check if target bitmap is of the same size as the MATLAB RGB array
    if (height != - pBitmapInfo->bmiHeader.biHeight || width != pBitmapInfo->bmiHeader.biWidth)
    {
        return E_INVALIDARG;
    }

    // Allocate space for the bitmap data
    *bitmapBits = new BYTE[height * width * PIXEL_BYTE_SIZE];
    if (!(*bitmapBits))
    {
        return E_OUTOFMEMORY;
    }

    BYTE* bits = reinterpret_cast<BYTE*>(*bitmapBits);
    BYTE* matlabData = reinterpret_cast<BYTE*>(mxGetData(pImg));

    // Convert from MATLAB matrix to Windows GDI bitmap
    for (int y = 0 ; y < height ; y += 1)
    {
        for (int x = 0 ; x < width ; x += 1) 
        {
            BYTE* pixel = bits + (x + y * width) * PIXEL_BYTE_SIZE;
            *pixel = *(matlabData + y + x * height + 2 * width * height);			// Blue pixel
            *(pixel + 1) = *(matlabData + y + x * height + width * height);			// Green pixel
            *(pixel + 2) = *(matlabData + y + x * height);							// Red pixel
            *(pixel + 3) = 0;														// Not used byte
        }
    }

    return S_OK;
}

/// <summary>
/// Validates an RGB MATLAB mxArray
/// </summary>
/// <param name="pImg">pointer to mxArray holding image to validate</param>
/// <returns>S_OK if valid, an error code otherwise</returns>
HRESULT MatlabHelper::ValidateMxArrayRgbImage(const mxArray* pImg)
{
    HRESULT hr = S_OK;

    // Fail if pointer is invalid
    if (!pImg) 
    {
        hr = E_POINTER;
    }

    // Fail if matrix contains no data or does not contain RGB data
    if (mxIsEmpty(pImg) || !mxIsUint8(pImg) || mxGetNumberOfDimensions(pImg) != RGB_DIMENSIONS) 
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

/// <summary>
/// Converts a MATLAB return code into an HRESULT
/// </summary>
/// <param name="retCode">MATLAB return code</param>
/// <returns>S_OK if valid return code indicates success, E_FAIL if an error occurred</returns>
HRESULT MatlabHelper::ConvertMatlabRetCodeToHResult(int retCode)
{
    //	MATLAB only returns two return codes: 0 for success
    //										  1 for error
    //	When MATLAB returns a 1, it does not specify what the error is
    HRESULT hr = E_FAIL;

    if (retCode == 0) 
    {
        hr = S_OK;
    }

    return hr;
}

/// <summary>
/// Puts a MATLAB matrix (mxArray) into the MATLAB engine environment
/// </summary>
/// <param name="name">name of the matrix</param>
/// <param name="pVariable">pointer to matrix to put into environment</param>
/// <returns>S_OK if variable placed in MATLAB, an error code otherwise</returns>
HRESULT MatlabHelper::MatlabPutVariable(const char* name, const mxArray* pVariable)
{
    if (!name || !pVariable)
    {
        return E_POINTER;
    }

    int retCode = engPutVariable(m_matlabEngine, name, pVariable);

    return ConvertMatlabRetCodeToHResult(retCode);
}

/// <summary>
/// Gets a MATLAB matrix (mxArray) from the MATLAB engine environment
/// </summary>
/// <param name="name">name of the matrix</param>
/// <param name="ppVariable">pointer to update with location of fetched matrix</param>
/// <returns>S_OK if variable fetched from MATLAB, an error code otherwise</returns>
HRESULT MatlabHelper::MatlabGetVariable(const char* name, mxArray** ppVariable)
{
    if (!name || !ppVariable)
    {
        return E_POINTER;
    }

    mxArray* pVar = engGetVariable(m_matlabEngine, name);
    if (!pVar) {
        // engGetVaraible only returns NULL if the variable does not exist in the MATLAB environment
        return E_NOT_SET;					
    }
    *ppVariable = pVar;

    return S_OK;
}

/// <summary>
/// Sends an expression to MATLAB for evaluation
/// </summary>
/// <param name="expr">expression string to evaluate</param>
/// <returns>S_OK if expression sent to MATLAB, an error code otherwise</returns>
HRESULT MatlabHelper::MatlabEvalExpr(const char* expr)
{
    if (!expr)
    {
        return E_POINTER;
    }

    int retCode = engEvalString(m_matlabEngine, expr);

    return ConvertMatlabRetCodeToHResult(retCode);
}

//// <summary>
/// Performs greyscale threshold
/// </summary>
/// <param name="pImg">pointer to the image that will have a filter applied to it</param>
/// <returns>S_OK if success, E_FAIL if an error occurred</returns>
HRESULT MatlabHelper::ApplyGrayscaleThreshold(mxArray* pImg)
{
	HRESULT hr;

	hr = MatlabPutVariable("img", pImg);
	if (FAILED(hr))
	{
		return hr;
	}

	// Convert image to grayscale
	const char* c_convertToGrayscaleExpr = "[labels, centers] = imsegkmeans(img,5);";
	hr = MatlabEvalExpr(c_convertToGrayscaleExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	//// Find threshold of background for image
	//const char* c_findThresholdExpr = "threshold = graythresh(grayscale_img);";
	//hr = MatlabEvalExpr(c_findThresholdExpr);
	//if (FAILED(hr))
	//{
	//	return hr;
	//}

	//// Compute binary mask to reflect thresholding of grayscale image
	//const char* c_computeBinaryMaskExpr = "binary_mask = uint8(~imbinarize(grayscale_img, threshold));";
	//hr = MatlabEvalExpr(c_computeBinaryMaskExpr);
	//if (FAILED(hr))
	//{
	//	return hr;
	//}

	// Apply binary mask to original image
	const char* c_applyBinaryMaskExpr = "filtered_img = labeloverlay(img, labels);";
	hr = MatlabEvalExpr(c_applyBinaryMaskExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	// Get back filtered image
	mxArray* pFilteredImage;
	hr = MatlabGetVariable("filtered_img", &pFilteredImage);
	if (FAILED(hr))
	{
		return hr;
	}

	// Overwrite passed in image with the filtered image
	hr = MoveRgbMxArrayData(pFilteredImage, pImg);
	mxDestroyArray(pFilteredImage);

	return hr;
}

//// <summary>
/// Finds centroids of objects on largest portion of background
/// </summary>
/// <param name="pImg">pointer to the image that will have a filter applied to it</param>
/// <param name="centroids">2D vector array that contains the centroids of each object</param>
/// <returns>S_OK if success, E_FAIL if an error occurred</returns>
HRESULT MatlabHelper::ComputeCentroids(mxArray* pImg, std::vector<std::vector<double>> centroids)
{
	HRESULT hr;

	hr = MatlabPutVariable("img", pImg);
	if (FAILED(hr))
	{
		return hr;
	}

	const char* c_removeBackgroundExpr = "grayscale_img = rgb2gray(img); \
											threshold = graythresh(grayscale_img); \
											only_background_mask = imbinarize(grayscale_img, threshold); \
											not_background_mask = ~only_background_mask; \
											no_background_img = img.*repmat(uint8(not_background_mask), [1, 1, 3]);";
	hr = MatlabEvalExpr(c_removeBackgroundExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	const char* c_findTableExpr = "only_background_mask = imfill(only_background_mask, 'holes'); \
									background_regions = regionprops(only_background_mask, grayscale_img, { 'Area', 'Centroid', 'PixelIdxList' }); \
									[max_area, max_id] = max([background_regions.Area]); \
									max_region_pixels = background_regions(max_id).PixelIdxList;";
	hr = MatlabEvalExpr(c_findTableExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	const char* c_restrictToTableExpr = "table_mask = zeros(size(only_background_mask)); \
											table_mask(max_region_pixels) = 1; \
											table_mask = imerode(table_mask, strel('cube', 10)); \
											only_table_img = no_background_img.*repmat(uint8(table_mask), [1, 1, 3]); \
											no_table_background_mask = table_mask & not_background_mask;";
	hr = MatlabEvalExpr(c_restrictToTableExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	const char* c_findCentroidsExpr = "objects = regionprops(no_table_background_mask,grayscale_img,{'Area', 'Centroid'}); \
										allCentroids = arrayfun(@(n) n > 60, [objects.Area]); \
										validCentroidIdx = find(allCentroids == 1); \
										numValidCentroids = length(validCentroidIdx); \
										unformattedCentroids = [objects(validCentroidIdx).Centroid]; \
										finalCentroids = transpose(reshape(unformattedCentroids, [2, numValidCentroids]));";
	hr = MatlabEvalExpr(c_findCentroidsExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	const char* c_addCentroidsExpr = "filteredImg = img; \
											for i=1:numValidCentroids \
												currX = finalCentroids(i, 1); \
												currY = finalCentroids(i, 2); \
												filteredImg = insertShape(filteredImg, 'circle', [currX currY 20], 'LineWidth', 10, 'Color', 'green'); \
											end";
	hr = MatlabEvalExpr(c_addCentroidsExpr);
	if (FAILED(hr))
	{
		return hr;
	}

	// Get back filtered image
	mxArray* pFilteredImage;
	hr = MatlabGetVariable("filteredImg", &pFilteredImage);
	if (FAILED(hr))
	{
		return hr;
	}

	// Get back filtered image
	mxArray* pCentroids;
	hr = MatlabGetVariable("finalCentroids", &pCentroids);
	if (FAILED(hr))
	{
		return hr;
	}

	// Overwrite passed in image with the filtered image
	hr = MoveRgbMxArrayData(pFilteredImage, pImg);
	mxDestroyArray(pFilteredImage);

	return hr;
}

/// <summary>
/// Moves RGB data from one mxArray to another
/// </summary>
/// <param name="pSourceImg">pointer to the source image</param>
/// <param name="pDestImg">pointer to the destination image</param>
/// <returns>S_OK if move is successful, E_FAIL if an error occurred</returns>
HRESULT MatlabHelper::MoveRgbMxArrayData(mxArray* pSourceImg, mxArray* pDestImg)
{
    // Check that destination image can hold data from source image
    size_t sourceElementSize = mxGetElementSize(pSourceImg);
    size_t numSourceDimensions = mxGetNumberOfDimensions(pSourceImg);
    const mwSize* sourceDimensions = mxGetDimensions(pSourceImg);
    size_t destElementSize = mxGetElementSize(pDestImg);
    size_t numDestDimensions = mxGetNumberOfDimensions(pDestImg);
    const mwSize* destDimensions = mxGetDimensions(pDestImg);

    if (sourceElementSize != destElementSize || numSourceDimensions != 3 || numSourceDimensions != numDestDimensions
        || sourceDimensions[0] != destDimensions[0] || sourceDimensions[1] != destDimensions[1] || sourceDimensions[2] != destDimensions[2])
    {
        return E_INVALIDARG;
    }

    // Move the data over
    void* pDestImgData = mxGetData(pDestImg);
    mxFree(pDestImgData);
    void* pSourceImgData = mxGetData(pSourceImg);
    mxSetData(pDestImg, pSourceImgData);
    mxSetData(pSourceImg, NULL);

    return S_OK;
}