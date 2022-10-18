//include appropriate header files
#include "itkImage.h"
#include "itkMeanSampleFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <itkN4BiasFieldCorrectionImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>
#include <itkStatisticsImageFilter.h>

int
main(int argc, char* argv[]) {
    if (argc != 7) { //if not 7 arguments, print error.
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0]; //first argument is program name
        std::cerr << " <InputFileName> <OutputFileName> <numberOfIterations> <timeStep> "
            "<conductance> <referenceImageName>";
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    //After trial and error, we found that the best values for number of iterations is 20, timeStep is .06 and conductance is 3
    //for the command line argument inputs that go into the anisotropic image filter.

    //read arguments into appropriate variables
    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];
    const int numberOfIterations = std::stoi(argv[3]);
    const float timeStep = std::stod(argv[4]);
    const float conductance = std::stod(argv[5]);
    const char* ReferenceImageFileName = argv[6];

    const int Dimension = 3; //we are using 3D images
    using InputPixelType = float;
    using InputImageType = itk::Image<InputPixelType, Dimension>;
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    //instantiate reader and read in input image
    using ReaderType = itk::ImageFileReader<InputImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputFileName);

    //instantiate anisotropic image filter and pass in image data from reader
    using FilterType = itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InputImageType>;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(reader->GetOutput());
    std::cout << "Read" << std::endl;
    filter->SetNumberOfIterations(numberOfIterations);
    filter->SetTimeStep(timeStep);
    filter->SetConductanceParameter(conductance);
    filter->Update();

    //instantiate n3(called n4 in ITK) image filter and pass in image data from anisotropic filter object
    using N4FilterType = itk::N4BiasFieldCorrectionImageFilter<InputImageType, InputImageType>;
    N4FilterType::Pointer n4pointer = N4FilterType::New();
    n4pointer->SetInput(filter->GetOutput());
    std::cout << "Denoising finished" << std::endl;

    //instantiate Histogram Matching Image Filter and pass in image data from n3/n4 image filter object
    using HistogramMatchingImageFilterType = itk::HistogramMatchingImageFilter<InputImageType, InputImageType>;
    HistogramMatchingImageFilterType::Pointer hmPointer = HistogramMatchingImageFilterType::New();
    hmPointer->SetInput(n4pointer->GetOutput());
    std::cout << "N3 finished" << std::endl;
    //read in reference image and pass it to Histogram match filter object
    InputImageType::Pointer image = itk::ReadImage<InputImageType>(ReferenceImageFileName); //read in reference image
    hmPointer->SetReferenceImage(image);
    hmPointer->Update();

    // instantite rescaler and pass in data from histogram matcher object to make imge data compatible format with writer object
    using RescaleType = itk::RescaleIntensityImageFilter<InputImageType, OutputImageType>;
    RescaleType::Pointer rescaler = RescaleType::New();
    rescaler->SetInput(hmPointer->GetOutput());
    std::cout << "Histogram matching finished" << std::endl;
    rescaler->SetOutputMinimum(itk::NumericTraits<OutputPixelType>::min());
    rescaler->SetOutputMaximum(itk::NumericTraits<OutputPixelType>::max());

    //instantiate writer object, get data from rescaler object
    using WriterType = itk::ImageFileWriter<OutputImageType>;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputFileName);
    writer->SetInput(rescaler->GetOutput());
    std::cout << "Rescaling finished" << std::endl;
    //try writing to output file, else throw error
    try {
        writer->Update();
    }
    catch (itk::ExceptionObject& error) { //
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    //instantiate reader and read reference image
    using ReaderType = itk::ImageFileReader<InputImageType>;
    ReaderType::Pointer reader2 = ReaderType::New();
    reader2->SetFileName(ReferenceImageFileName);

    //pass reference image to StatisticsFilterType object after instantiation.
    // Call sigma and mean function, divide to get CoV and print out.
    using StatisticsFilterType = itk::StatisticsImageFilter<InputImageType>;
    StatisticsFilterType::Pointer statisticsPointer = StatisticsFilterType::New();
    statisticsPointer->SetInput(reader2->GetOutput());
    statisticsPointer->Update();
    std::cout << "CoV of Reference Image: " << statisticsPointer->GetSigma() / statisticsPointer->GetMean() << std::endl;


    //pass the orginal/input image to StatisticsFilterType object after instantiation.
    // Call sigma and mean function, divide to get CoV and print out.
    reader2->SetFileName(inputFileName);
    statisticsPointer->SetInput(reader2->GetOutput());
    statisticsPointer->Update();
    std::cout << "CoV of Original Image: " << statisticsPointer->GetSigma() / statisticsPointer->GetMean() << std::endl;


    //pass output image to StatisticsFilterType object.
    // Call sigma and mean function, divide to get CoV and print out.
    reader2->SetFileName(outputFileName);
    statisticsPointer->SetInput(reader2->GetOutput());
    statisticsPointer->Update();
    std::cout << "CoV of Output Image: " << statisticsPointer->GetSigma() / statisticsPointer->GetMean() << std::endl;


    return EXIT_SUCCESS;
}
