{
    "targets": [
        {
            "target_name": "iastnativemethods",
            "sources": [
                "./src/IAST.cc"
            ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ],
            "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
            "cflags_cc!": [ "-fno-exceptions", "-fno-rtti"  ],
            "cflags_cc": [ "-std=c++17" ],
            "conditions": [
                ['OS=="mac"', {
                    "xcode_settings": {
                        "MACOSX_DEPLOYMENT_TARGET": "10.7",
                        "CLANG_CXX_LIBRARY": "libc++",
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "GCC_ENABLE_CPP_RTTI": "YES"
                    }
                }]
            ],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "defines": [
                'NODE_VERSION_<!(echo $NODE_VERSION)=1'
            ]
        },
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": ["iastnativemethods"],
            "copies": [
                {
                    "files": [ "<(PRODUCT_DIR)/iastnativemethods.node" ],
                    "destination": "dist/"
                }
            ]
        }
    ]
}
