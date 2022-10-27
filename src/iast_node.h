/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_IAST_NODE_H_
#define SRC_IAST_NODE_H_

#if defined(NODE_VERSION_v16)
#define IS_EXTERNAL()   IsExternalTwoByte()
#else
#define IS_EXTERNAL()   IsExternal()
#endif

#endif  // SRC_IAST_NODE_H_
