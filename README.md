# Biomedical-Image-Analysis

This is an image filter pipeline that takes as 2 MRI files, one reference and one to to be altered. 

The image will:

1. be cleaned of high frequency noise with CurvatureAnisotropicDiffusionImageFilter

2. be cleaned of low frequency noise from the MRI machine through using the N3/N4 filter, and

3. have its pixel intensity histogram matched to that of the refence image's using the HistogramMatchingImageFilterType

 

![alt text](https://github.com/JWu-Git/Biomedical-Image-Analysis/blob/main/Result.png)
