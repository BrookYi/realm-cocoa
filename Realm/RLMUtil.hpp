////////////////////////////////////////////////////////////////////////////
//
// Copyright 2014 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#import <Realm/RLMConstants.h>
#import <objc/runtime.h>

#import <realm/binary_data.hpp>
#import <realm/string_data.hpp>

@class RLMObjectSchema;
@class RLMProperty;
@class RLMRealm;
@class RLMSchema;

// Helper structs for unretained<> to make unretained<id> work
template<typename T>
struct RLMUnretainedPtr {
    using type = __unsafe_unretained T *const;
};

template<>
struct RLMUnretainedPtr<id> {
    using type = __unsafe_unretained id const;
};

// type alias for const unretained pointers
// only needs to be used in function/method definitions, not declarations
template<typename T>
using unretained = typename RLMUnretainedPtr<T>::type;

NSException *RLMException(NSString *message, NSDictionary *userInfo = nil);
NSException *RLMException(std::exception const& exception);

NSError *RLMMakeError(RLMError code, std::exception const& exception);

void RLMSetErrorOrThrow(NSError *error, NSError **outError);

// returns if the object can be inserted as the given type
BOOL RLMIsObjectValidForProperty(id obj, RLMProperty *prop);

// returns a validated object for an input object
// creates new objects for child objects and array literals as necessary
// throws if passed in literals are not compatible with prop
id RLMValidatedObjectForProperty(id obj, RLMProperty *prop, RLMSchema *schema);

// throws if the values in array are not valid for the given schema
// returns array with allocated child objects
NSArray *RLMValidatedArrayForObjectSchema(NSArray *array, RLMObjectSchema *objectSchema, RLMSchema *schema);

// gets default values for the given schema (+defaultPropertyValues)
// merges with native property defaults if Swift class
NSDictionary *RLMDefaultValuesForObjectSchema(RLMObjectSchema *objectSchema);

// throws if the values in dict or properties in a kvc object are not valid for the given schema
// inserts default values for missing properties when allowMissing is false
// throws for missing properties when allowMissing is false
// returns dictionary with default values and allocates child objects when applicable
NSDictionary *RLMValidatedDictionaryForObjectSchema(id value, RLMObjectSchema *objectSchema, RLMSchema *schema, bool allowMissing = false);

NSArray *RLMCollectionValueForKey(NSString *key, RLMRealm *realm, RLMObjectSchema *objectSchema, size_t count, size_t (^indexGenerator)(size_t index));

void RLMCollectionSetValueForKey(id value, NSString *key, RLMRealm *realm, RLMObjectSchema *objectSchema, size_t count, size_t (^indexGenerator)(size_t index));

// C version of isKindOfClass
static inline BOOL RLMIsKindOfClass(Class class1, Class class2) {
    while (class1) {
        if (class1 == class2) return YES;
        class1 = class_getSuperclass(class1);
    }
    return NO;
}

// Returns whether the class is an indirect descendant of RLMObjectBase
BOOL RLMIsObjectSubclass(Class klass);

template<typename T>
static inline T *RLMDynamicCast(__unsafe_unretained id obj) {
    if ([obj isKindOfClass:[T class]]) {
        return obj;
    }
    return nil;
}

// Translate an rlmtype to a string representation
static inline NSString *RLMTypeToString(RLMPropertyType type) {
    switch (type) {
        case RLMPropertyTypeString:
            return @"string";
        case RLMPropertyTypeInt:
            return @"int";
        case RLMPropertyTypeBool:
            return @"bool";
        case RLMPropertyTypeDate:
            return @"date";
        case RLMPropertyTypeData:
            return @"data";
        case RLMPropertyTypeDouble:
            return @"double";
        case RLMPropertyTypeFloat:
            return @"float";
        case RLMPropertyTypeAny:
            return @"any";
        case RLMPropertyTypeObject:
            return @"object";
        case RLMPropertyTypeArray:
            return @"array";
    }
    return @"Unknown";
}

// String conversion utilities
static inline NSString * RLMStringDataToNSString(realm::StringData stringData) {
    static_assert(sizeof(NSUInteger) >= sizeof(size_t),
                  "Need runtime overflow check for size_t to NSUInteger conversion");
    return [[NSString alloc] initWithBytes:stringData.data()
                                    length:stringData.size()
                                  encoding:NSUTF8StringEncoding];
}

static inline realm::StringData RLMStringDataWithNSString(NSString *string) {
    static_assert(sizeof(size_t) >= sizeof(NSUInteger),
                  "Need runtime overflow check for NSUInteger to size_t conversion");
    return realm::StringData(string.UTF8String,
                               [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
}

// Binary convertion utilities
static inline realm::BinaryData RLMBinaryDataForNSData(NSData *data) {
    return realm::BinaryData(static_cast<const char *>(data.bytes), data.length);
}
