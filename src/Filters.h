#pragma once
#include <glad/glad.h>
#include <cstring>
#include "FilesystemAdapter.h"
#include "KnownMask.h"
#include "ImageWindowState.h"
#define _USE_MATH_DEFINES
#include <math.h>

struct ImageWindowStateVideo;

void InitVertexBuffer();

/**
 * Every filter does the following:
 * * Initializes the shader.
 * * Keeps track of necessary state for each shader.
 * * Render UI controls.
 *
 **/
struct IFilter {
	int _width;
	int _height;

	bool _enabled = true;
	char * _name;
	GLuint _programID;
	GLuint _outputFramebuffer;
	GLuint _outputTexture;
	virtual void InitShader() = 0;
	virtual void RenderUI() = 0;
	virtual void ApplyFilter(GLuint prevTexture) = 0;

	GLuint Draw(GLuint prevTexture)
	{
	    if(!_enabled) {
	        return prevTexture;
	    }

		glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
		glViewport(0,0,_width,_height);
		glUseProgram(_programID);

		ApplyFilter(prevTexture);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return _outputTexture;
	}
};

struct MainFilter: IFilter {
	GLuint _textureSampler;

	MainFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Main";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct VideoFilter: IFilter {
    GLuint _textureSampler;
	ImageWindowStateVideo* videoState;
    bool isPlaying = false;


	VideoFilter(int w, int h, ImageWindowStateVideo* video_ptr) {
		_width = w;
		_height = h;
		videoState = video_ptr;
		_name = "Video";
	}

    void InitShader() override;
    void RenderUI() override;
    void ApplyFilter(GLuint prevTexture) override;
};

struct SingleBandFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glBand;
	int _band = 0;

	SingleBandFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "SingleBand";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct SubstractionFilter: IFilter {
	GLuint _textureSampler;
	GLuint _secondTex;
	GLuint _secondSampler;
	GLuint _factor;
	GLuint _glMin, _glMax;
	bool _subtract;
	float _min[3] = {0}, _max[3] = {1, 1, 1};
	std::string _path;
	
	SubstractionFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Substraction";
		_path = fs::current_path().string();
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct NegativeFilter: IFilter {
	GLuint _textureSampler;
	NegativeFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Negative";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct ScalarFilter: IFilter {
	GLuint _textureSampler;
	GLuint _factorGl;
	GLuint _glC;
	GLuint _glDynamicRange;
	float _factor;

	bool _dynamicRange = false;
	ScalarFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Scalar";
		_factor = 1;
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct DynamicRangeCompressionFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glC;
	unsigned char _minr, _ming, _minb, _maxr, _maxg, _maxb;

	bool _calcToggle = true;
	DynamicRangeCompressionFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Dynamic range compression";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct GammaFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glGamma;

	float _gamma = 2;
	GammaFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Gamma";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct ThresholdFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glThreshold;
	float _threshold[3] = {0.5f, 0.5f, 0.5f};
	ThresholdFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Threshold";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct GlobalThresholdFilter: IFilter {
    GLuint _textureSampler;
    GLuint _glThreshold;
    bool _thresholdChanged = true;
    float _thresholdError = 0.1f;
    float _initialThreshold[3] = {0.5f, 0.5f, 0.5f};
    float _currentThreshold[3] = {0.5f, 0.5f, 0.5f};

    GlobalThresholdFilter(int w, int h) {
        _width = w;
        _height = h;
        _name = "Global Threshold";
    }

    void InitShader() override;
    void RenderUI() override;
    void ApplyFilter(GLuint prevTexture) override;
};

struct OtsuThresholdFilter: ThresholdFilter {

    bool _thresholdChanged = true;
    OtsuThresholdFilter(int w, int h): ThresholdFilter(w, h) { }

    void RenderUI() override;
    void ApplyFilter(GLuint prevTexture) override;
};

struct ContrastFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glLower, _glLowerTo;
	GLuint _glHigher, _glHigherTo;

	float _lower[3] = {0, 0, 0};
	float _lowerTo[3] = {0, 0, 0};
	float _higher[3]  = {1, 1, 1};
	float _higherTo[3]  = {1, 1, 1};

	ContrastFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Contrast";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct EqualizationFilter: IFilter {
	GLuint _textureSampler;
	GLuint _eqSampler;
	GLuint _eqTexture;
	bool _eqCalcTexture = true;

	EqualizationFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Equalization";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct ExponentialNoiseFilter: IFilter {
	GLuint _textureSampler;
	GLuint _randomTex;
	GLuint _randomSampler;
	GLuint _glLambda;
	GLuint _glContamination;
	int _seed = 0;
	float _lambda = 2;
	float _contamination = 0.1;
	ExponentialNoiseFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Exponential Noise Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct RayleighNoiseFilter: IFilter {
	GLuint _textureSampler;
	GLuint _randomTex;
	GLuint _randomSampler;
	GLuint _glXi;
	GLuint _glContamination;
	int _seed = 0;
	float _xi = 0.1;
	float _contamination = 0.1;
	RayleighNoiseFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Rayleigh Noise Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct GaussianNoiseFilter: IFilter {
	GLuint _textureSampler;
	GLuint _randomTex;
	GLuint _randomSampler;
	GLuint _randomTex2;
	GLuint _randomSampler2;
	GLuint _glSigma;
	GLuint _glMu;
	GLuint _glContamination;
	float _contamination = 0.1;
	int _seed[2] = {0, 100};
	float _sigma = 1;
	float _mu = 0;
	GaussianNoiseFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Gaussian Noise Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct SaltAndPepperNoiseFilter: IFilter {
	GLuint _textureSampler;
	GLuint _randomTex;
	GLuint _randomSampler;
	GLuint _glContamination1;
	GLuint _glContamination2;

	float _contamination1 = 0.01, _contamination2 = 0.9;
	int _seed = 0;
	
	SaltAndPepperNoiseFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Salt And Pepper Noise Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct CombinedMaskFilter: IFilter{
    GLuint _textureSampler;
    GLuint _glWidth, _glHeight;
    GLuint _glMaskSize1;
    GLuint _glMaskSize2;
    GLuint _glMaskSampler1;
    GLuint _glMaskSampler2;
    GLuint _maskWeightsTexture1;
    GLuint _maskWeightsTexture2;
    GLuint _glMaskDivision;

    int _maskSize1 = 3;
    int _maskSize2 = 3;
    float  * _weights1;
    float  * _weights2;
    float _sigma = 1;
    float _maskDivision = 9;

    CombinedMaskFilter(int w, int h, char* name) {
        _width = w;
        _height = h;
        _name = name;
        _weights1 = new float[100];
        for (int i = 0; i < 100; i++) {
            _weights1[i] = 1;
        }

        _weights2 = new float[100];
        for (int i = 0; i < 100; i++) {
            _weights2[i] = 1;
        }
    }

    void InitShader() override;
    void ApplyFilter(GLuint prevTexture) override;
    void InitMask();
};

struct MaskFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glMaskSize;
	GLuint _glWidth, _glHeight;
	GLuint _glMaskSampler;
	GLuint _maskWeightsTexture;
	GLuint _glMaskDivision;

	int _maskSize = 3;
	float  * _weights;
	float _sigma = 1;
	float _maskDivision = 9;

    MaskFilter(int w, int h, char* name) {
		_width = w;
		_height = h;
		_name = name;
		_weights = new float[100];
		for (int i = 0; i < 100; i++) {
			_weights[i] = 1;
		}
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
	void InitMask();
};

struct MeanFilter: MaskFilter {

    MeanFilter(int w, int h): MaskFilter(w, h, const_cast<char *>("Mean Filter")) { }

    void RenderUI() override;
};

struct BorderFilter: CombinedMaskFilter {

    bool _showSecondMask = true;
		float _sigma = 1.0f;

    BorderFilter(int w, int h): CombinedMaskFilter(w,h, const_cast<char *>("BorderFilter")) {
        _maskDivision = 1;
        setupSobelHorizontal(&_maskSize1, _weights1);
        setupSobelVertical(&_maskSize2, _weights2);
    }

    void RenderUI() override;
};

struct MedianFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glMaskSize;
	GLuint _glWidth, _glHeight;
	GLuint _glMaskSampler;
	GLuint _maskWeightsTexture;
	GLuint _glMaskDivision;

	int _maskSize = 3;
	float  * _weights;
	float _maskDivision = 9;
	
	MedianFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Median Filter";
		_weights = new float[100];
		for (int i = 0; i < 100; i++) {
			_weights[i] = 1;
		}
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
	void InitMask();
};

enum BorderDetectorType {
	NONE = 0,
	LECLERC = 1,
	LORENTZIANO = 2
};

struct HeatFilter: IFilter {
	GLuint _textureSampler;
	GLuint _textureSamplerBis;
    GLuint _textureBis;
    GLuint _frameBufferBis;

	GLuint _glBorderDetector;
	GLuint _glSigma;
	GLuint _glWidth;
	GLuint _glHeight;

	BorderDetectorType _borderDetectorType = NONE;

	float _sigma = 0.3;
	int iterations = 1;
	bool hasChanged = true;

	HeatFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Heat Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;


};

struct LaplaceFilter: IFilter {
	GLuint _firstPassTextureSampler;
	GLuint _firstPassProgramID;
	GLuint _glWidthFirstPass, _glHeightFirstPass;
	GLuint _textureSampler;
	GLuint _firstPassTexture;
	GLuint _firstPassFrameBuffer;
	GLuint _glThreshold;
	GLuint _glMax2;

	GLuint _glMaskSize;
	GLuint _glMaskSampler;
	GLuint _maskWeightsTexture;
	GLuint _glWidth;
	GLuint _glHeight;
	GLuint _glMax;

	float _threshold = 0;
	int _maskSize = 3;
	float * _weights;
	float _sigma = 1;
	float _max = 0;

	LaplaceFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Laplace Filter";
		_weights = new float[100];
		float weights[] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
		_max = 4;
		memcpy(_weights, weights, sizeof(float) * _maskSize * _maskSize);
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
	void InitMask();
};

struct BilateralFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glMaskSize;	
	GLuint _glWidth;
	GLuint _glHeight;
	GLuint _glSigmaS;
	GLuint _glSigmaR;

	int _maskSize = 7;
	float _sigmaS = 2, _sigmaR = 30;

	BilateralFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Bilateral Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct SusanFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glEpsilon;
	GLuint _glEdgeDetectionCutoff;

	float _epsilon = 0.05f;
	float _edgeDetectionCutoff = 0.1f;

	SusanFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Susan Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct CannyFilter: IFilter {
	GLuint _textureSampler;
	GLuint _glEpsilon;
	GLuint _glEdgeDetectionCutoff;
	GLuint _glT1;
	GLuint _glT2;
	float _epsilon = 0.05f;
	float _edgeDetectionCutoff = 0.1f;
	float _t1 = 0.1f;
	float _t2 = 0.9f;

	CannyFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Canny Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct HughFilter: IFilter {
	GLuint _textureSampler;
	GLuint _drawnText;
	GLuint _lastPrevTexture;
	unsigned char * _pixels = nullptr;
	unsigned char * _pixelsOut = nullptr;
	int * _acumulator = nullptr;
	int _ro_discr;
	int _theta_discr;
	float _epsilon = 0.6;
	int _cutoff = 10;
	// float _theta[2] = {-M_PI_2, M_PI_2};
	// float _

	HughFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Hugh Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
	void BuildAcumulator();
	void ResetPixelsOut();
};

struct HughCircleFilter: IFilter {
	GLuint _textureSampler;
	GLuint _drawnText;
	GLuint _lastPrevTexture;
	unsigned char * _pixels = nullptr;
	unsigned char * _pixelsOut = nullptr;
	int * _acumulator = nullptr;
	int _width_discr;
	int _height_discr;
	int _r_discr;
	float _epsilon = 40;
	int _cutoff = 50;
	// float _theta[2] = {-M_PI_2, M_PI_2};
	// float _

	HughCircleFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Hugh Circle Filter";
	}

	void InitShader() override;
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
	void BuildAcumulator();
	void ResetPixelsOut();
};

struct ActiveBorder: IFilter {
	GLuint _levelValueProgramId;

	GLuint _levelValueFrameBuffer;

	GLuint _levelValueTexture;
	GLuint _maskWeightsTexture;

	float* _levelValues;

	int _iterations = 0;
	bool _showSquare = true;

	float _precision = 0.5f;

	int _xs[2] = {255, 313};
	int _ys[2] = {137, 165};

	int _kSize = 30;
	int* _kc;
	int _kcI = 0;
	int _kcF = 0;
	float _umbral = 0.3f;

	float _lastCenter[2] = {0,0};

	float _medianColorValue[3] = {0,0,0};
	float _resizeSquareRadius = 100;

	ActiveBorder(int w, int h) {
		_width = w;
		_height = h;
		_name = "Active Border Filter";

		_levelValues = new float[_width * _height]();
        _kc = new int[_kSize]();
	}

	void InitShader() override;
	void SetupShader();
	void ApplySquare(GLuint prevTexture, int xmin, int xmax, int ymin, int ymax, bool recalculateColor);
	void RenderUI() override;
	void ApplyFilter(GLuint prevTexture) override;
};

struct HarrisFilter: IFilter {

	GLuint _maskProgramId;
	GLuint _lxFrameBuffer;
	GLuint _lxTexture;

	GLuint _lyFrameBuffer;
	GLuint _lyTexture;

	GLuint _multProgramId;

	GLuint _lx2FrameBuffer;
	GLuint _lx2Texture;

	GLuint _ly2FrameBuffer;
	GLuint _ly2Texture;

	GLuint _lxyFrameBuffer;
	GLuint _lxyTexture;

	GLuint _cimProgramId;
	GLuint _cimFrameBuffer;
	GLuint _cimTexture;

	GLuint _pxmaskWeightsTexture;
	GLuint _pymaskWeightsTexture;
	GLuint _gaussmaskWeightsTexture;

	float  * _pxweights;
	float  * _pyweights;
	float  * _guassweights;

	float _prewitMaskDivision = 1.0f;
	float _gaussMaskDivision;
	float _threshold = 0.001f;
	float _cimV = 1;
	float _k = 0.04f;
    float _color[3] = {1.0f, 1.0f, 0};

	HarrisFilter(int w, int h) {
        _width = w;
        _height = h;
        _name = "Harris";

		_pxweights = new float[3*3];
		_pyweights = new float[3*3];
		_guassweights = new float[7*7];
    }

    void InitShader() override;
    void RenderUI() override;
    void SetupPrewitt(bool isX);
    void SetupGauss();
    void ApplyFilter(GLuint prevTexture) override;
};