X Merge new vigra
X .cvsignore files
X integer math dithering
X skipsm reduce replacement
    X vector
    X extrapolation
    X wraparound boundary conditions
    X no alpha
    X comment
X skipsm expand replacement
    X vector
    X normal boundary condition
    X wraparound boundary condition
    X comment
X fast cfi row iterators
    left-middle test before changes: 59.54
    left-middle test after changes: 50.50
    2.5 non-profiling test: 12.9 user
    3.0 non-profiling test: 7.24 user
X make checkpointing optional
    X exportImageAlpha needs accessor to convert AlphaType to ImageComponentType
      lmr test with checkpointing: 104.77
      lmr test with no checkpointing: 99.15
X vigra sized_int
X defines for choosing CachedFileImages vs. BasicImages.
X add GimpAssociatedAlphaHack to new vigra_impex
X EnblendNumericTraits
X icc blending
    X make sure all input images have same icc profile
    X do convert in image->pyramid pixel convert functors
    X exp2 or shift and multiply?
        exp2: 18.26 convert vector to JCH
        shift: 18.60 convert
    X progress indicators for color transforms
X add NDEBUG to Makefile.am
X fast fromRealPromote
    linux before: lm test 14.51 14.54 14.52
    linux after:  lm test 14.40 14.45 14.41
    win32 before: lm test 5.164 5.046 5.077
    win32 after:  lm test 5.154 5.005 4.967
    linux noprof: lm test 5.71 5.71 5.71
    linux 2.5:    lm test 13.01 12.97 12.96
X new seam alg
    X stride 8 mask generation
        X cfi strided iterators 99.15 -> 97.58
    X stride 8 mask polygonization
        - make it work with wraparound?
        X Arrange snakes into continuous pieces
        X Experiment with different distances between vertices
    X cost image
        X Replace with CachedFileImage?
        X Dijkstra vertices on BasicImage regions-of-interest
    X Strategy 1: GDA
        X integer math / bresenham LineIterator
        X Memoize magnitude 33.28/67.95 -> 33.20/67.85
        X Fast EXP approximation 25.95->17.18
        X factor out tCurrent divide 12.35
        X E integer 11.29
    X Strategy 2: Shortest Path
    X code cleanup and comments
    X progress indicators
    X option to revert to old mask algorithm

- gpu blend
    - libsh
        - cpu blending speed: lr test  .04 self 2.79 children
        - gpu from_float lrint:        .11 self 3.52 children
                                      2.54 profile_to_float
                                       .92 profile_from_float
    - brook
        - cpu blending speed: lr test  .00 self 2.53 children
        - brook ogl backend:           .05 self 4.05 children
                                                2.75 profile_to_float
                                                1.27 profile_from_float
                                                 .03 gpuBlendKernel
                                        1.16 copyToTextureFormat
                                         .66 copyFromTextureFormat
                                        1.06 misc brook stuff

- brook GDA kernel
        - GDA no profiling:             5.74 user  .25 system  7.57 elapsed
                                        5.76 user  .25 system  7.36 elapsed
                                        5.70 user  .25 system  7.34 elapsed

        - GDA with gpu backend         23.82 user 5.81 system 32.48 elapsed
                                       23.68 user 5.84 system 32.27 elapsed
                                       23.56 user 5.74 system 31.78 elapsed

        - GDA with cpu backend         21.22 user  .26 system 23.56 elapsed
                                       21.22 user  .26 system 23.29 elapsed
                                       21.21 user  .26 system 23.78 elapsed

        - GDA with brook profiling      .20  33.61 iterate
                                       3.26  30.35 calculateStateProbabilitiesGPU
                                       5.83    .11 costImageCost
                                        .31  24.10 cSPGPU_core

- gl gpu gda kernel
    KMAX=32
        lr test gpu run= 4,03 costImageCost=3.52 calcStateProbs=0.19 kernel=0.00
        lr test cpu run= 6.03 costImageCost=3.51 calcStateProbs=2.02
    KMAX=64
        lr test gpu run= 9.80 costImageCost=8.81 calcStateProbs=0.50 kernel=0.00
        lr test cpu run=17.51 costImageCost=8.97 calcStateProbs=8.24

X replace EnblendROI with vigra::Rect2D
X load/save mask from file
X update copyright dates
X check that automatic level determination works correctly
X fix memory estimation messages

X test for black row bug in alpine dam photo
X wraparound tests
X testing, testing, testing
X ctrl-c cleans up temp files right

- test large files on windows
- vogl's hdr test cases
    - cinepaint crashes loading these images
- fix gpu second iteration nan bug

X enable all pixel types
X update version number
X merge into trunk
X build windows executable
    X compile libtiff with 64-bit file i/o api
    X link other vigra-impex graphics libraries
    X check libtiff warnings
- create release packages
- documentation update
    - man page
    - web page

- Stuff for later revisions:
    - make striding argument object factories for BasicImages
    - blending against nothing leads to output with alpha=0?
    - improve GDA state space enumeration
    - vigra impex improvements
    - cfi swap policy improvements
    - vigra multilayer tiff support
    - blend nadir and zenith in 360x360 panos.
    - make assemble smarter.
    - store alpha masks in RLE format
    - update to new vigra, use arg obj factories with ROI
        - add striding to arg obj factories with ROI
