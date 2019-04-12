/** @file
  APIs for JSON operations.

  Copyright (c) 2018 - 2019, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseJsonLib.h>
#include "JsonUtilities.h"

/**
  The function is used to convert a NULL terminated UTF8 encoded string to a JSON
  value. Only object and array represented strings can be converted successfully,
  since they are the only valid root values of a JSON text for UEFI usage.

  Real number and number with exponent part are not supportted by UEFI.

  Caller needs to cleanup the root value by calling JsonValueFree().

  @param[in]   Text             The NULL terminated UTF8 encoded string to convert

  @retval      Array JSON value or object JSON value, or NULL when any error occurs.

**/
EDKII_JSON_VALUE
EFIAPI
TextToJson (
  IN    CHAR8*    Text
  )
{
  json_error_t    JsonError;

  return (EDKII_JSON_VALUE) json_loads (Text, 0, &JsonError);
}

/**
  The function is used to convert the JSON root value to a UTF8 encoded string which
  is terminated by NULL, or return NULL on error.

  Only array JSON value or object JSON value is valid for converting, and caller is
  responsible for free converted string.

  @param[in]   Json               The JSON value to be converted

  @retval      The JSON value converted UTF8 string or NULL.

**/
CHAR8*
EFIAPI
JsonToText (
  IN    EDKII_JSON_VALUE    Json
  )
{
  if (!JsonValueIsArray (Json) && !JsonValueIsObject (Json)) {
    return NULL;
  }

  return json_dumps ((json_t *) Json, 0);
}

/**
  The function is used to initialize a JSON value which contains a new JSON array,
  or NULL on error. Initially, the array is empty.

  The reference count of this value will be set to 1, and caller needs to cleanup the
  value by calling JsonValueFree().

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @retval      The created JSON value which contains a JSON array or NULL.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitArray (
  VOID
  )
{
  return (EDKII_JSON_VALUE) json_array();
}

/**
  The function is used to initialize a JSON value which contains a new JSON object,
  or NULL on error. Initially, the object is empty.

  The reference count of this value will be set to 1, and caller needs to cleanup the
  value by calling JsonValueFree().

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @retval      The created JSON value which contains a JSON object or NULL.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitObject (
  VOID
  )
{
  return (EDKII_JSON_VALUE) json_object();
}

/**
  The function is used to initialize a JSON value which contains a new JSON string,
  or NULL on error.

  The input string must be NULL terminated Ascii format, non-Ascii characters will
  be processed as an error. Unicode characters can also be represented by Ascii string
  as the format: \u + 4 hexadecimal digits, like \u3E5A, or \u003F.

  The reference count of this value will be set to 1, and caller needs to cleanup the
  value by calling JsonValueFree().

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   String      The Ascii string to initialize to JSON value

  @retval      The created JSON value which contains a JSON string or NULL. Select a
               Getter API for a specific encoding format.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitAsciiString (
  IN    CHAR8    *String
  )
{
  UINTN    Index;

  if (String == NULL) {
    return NULL;
  }

  Index = 0;
  while (*(String + Index) != '\0') {
    if (((*(String + Index)) & 0x80) != 0x00) {
      return NULL;
    }

    Index ++;
  }

  return (EDKII_JSON_VALUE) json_string (String);
}

/**
  The function is used to initialize a JSON value which contains a new JSON string,
  or NULL on error.

  The input must be a NULL terminated UCS2 format Unicode string.

  The reference count of this value will be set to 1, and caller needs to cleanup the
  value by calling JsonValueFree().

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   String      The Unicode string to initialize to JSON value

  @retval      The created JSON value which contains a JSON string or NULL. Select a
               Getter API for a specific encoding format.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitUnicodeString (
  IN    CHAR16    *String
  )
{
  EFI_STATUS    Status;
  CHAR8         *Utf8Str;

  if (String == NULL) {
    return NULL;
  }

  Utf8Str = NULL;
  Status  = UCS2StrToUTF8 (String, &Utf8Str);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return (EDKII_JSON_VALUE) json_string (Utf8Str);
}

/**
  The function is used to initialize a JSON value which contains a new JSON integer,
  or NULL on error.

  The reference count of this value will be set to 1, and caller needs to cleanup the
  value by calling JsonValueFree().

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   Value       The integer to initialize to JSON value

  @retval      The created JSON value which contains a JSON number or NULL.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitNumber (
  IN    INT64    Value
  )
{
  return (EDKII_JSON_VALUE) json_integer (Value);
}

/**
  The function is used to initialize a JSON value which contains a new JSON boolean,
  or NULL on error.

  Boolean JSON value is kept as static value, and no need to do any cleanup work.

  @param[in]   Value       The boolean value to initialize.

  @retval      The created JSON value which contains a JSON boolean or NULL.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitBoolean (
  IN    BOOLEAN    Value
  )
{
  return (EDKII_JSON_VALUE) json_boolean (Value);
}

/**
  The function is used to initialize a JSON value which contains a new JSON NULL,
  or NULL on error.

  NULL JSON value is kept as static value, and no need to do any cleanup work.

  @retval      The created NULL JSON value.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueInitNull (
  VOID
  )
{
  return (EDKII_JSON_VALUE) json_null();
}

/**
  The function is used to decrease the reference count of a JSON value by one, and once
  this reference count drops to zero, the value is destroyed and it can no longer be used.
  If this destroyed value is object type or array type, reference counts for all containing
  JSON values will be decreased by 1. Boolean JSON value and NULL JSON value won't be destroyed
  since they are static values kept in memory.

  Reference Count Strategy: BaseJsonLib uses this strategy to track whether a value is still
  in use or not. When a value is created, it�s reference count is set to 1. If a reference
  to a value is kept for use, its reference count is incremented, and when the value is no
  longer needed, the reference count is decremented. When the reference count drops to zero,
  there are no references left, and the value can be destroyed.

  @param[in]   Json             The JSON value to be freed.

**/
VOID
EFIAPI
JsonValueFree (
  IN    EDKII_JSON_VALUE    Json
  )
{
  json_decref ((json_t *) Json);
}

/**
  The function is used to create a fresh copy of a JSON value, and all child values are deep
  copied in a recursive fashion. It should be called when this JSON value might be modified
  in later use, but the original still wants to be used in somewhere else.

  Reference counts of the returned root JSON value and all child values will be set to 1, and
  caller needs to cleanup the root value by calling JsonValueFree().

  * Note: Since this function performs a copy from bottom to up, too many calls may cause some
  performance issues, user should avoid unnecessary calls to this function unless it is really
  needed.

  @param[in]   Json             The JSON value to be cloned.

  @retval      Return the cloned JSON value, or NULL on error.

**/
EDKII_JSON_VALUE
EFIAPI
JsonValueClone (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return (EDKII_JSON_VALUE) json_deep_copy ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON value contains a JSON array.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value contains a JSON array.
  @retval      FALSE            The JSON value doesn't contain a JSON array.

**/
BOOLEAN
EFIAPI
JsonValueIsArray (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_array ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON value contains a JSON object.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value contains a JSON object.
  @retval      FALSE            The JSON value doesn't contain a JSON object.

**/
BOOLEAN
EFIAPI
JsonValueIsObject (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_object ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON Value contains a string, Ascii or
  Unicode format is not differentiated.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value contains a JSON string.
  @retval      FALSE            The JSON value doesn't contain a JSON string.

**/
BOOLEAN
EFIAPI
JsonValueIsString (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_string ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON value contains a JSON number.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value is contains JSON number.
  @retval      FALSE            The JSON value doesn't contain a JSON number.

**/
BOOLEAN
EFIAPI
JsonValueIsNumber (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_integer ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON value contains a JSON boolean.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value contains a JSON boolean.
  @retval      FALSE            The JSON value doesn't contain a JSON boolean.

**/
BOOLEAN
EFIAPI
JsonValueIsBoolean (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_boolean ((json_t *) Json);
}

/**
  The function is used to return if the provided JSON value contains a JSON NULL.

  @param[in]   Json             The provided JSON value.

  @retval      TRUE             The JSON value contains a JSON NULL.
  @retval      FALSE            The JSON value doesn't contain a JSON NULL.

**/
BOOLEAN
EFIAPI
JsonValueIsNull (
  IN    EDKII_JSON_VALUE    Json
  )
{
  return json_is_null ((json_t *) Json);
}

/**
  The function is used to retrieve the associated array in an array type JSON value.

  Any changes to the returned array will impact the original JSON value.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated array in JSON value or NULL.

**/
EDKII_JSON_ARRAY
EFIAPI
JsonValueGetArray (
  IN    EDKII_JSON_VALUE    Json
  )
{
  if (Json == NULL || !JsonValueIsArray (Json)) {
    return NULL;
  }

  return (EDKII_JSON_ARRAY) Json;
}

/**
  The function is used to retrieve the associated object in an object type JSON value.

  Any changes to the returned object will impact the original JSON value.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated object in JSON value or NULL.

**/
EDKII_JSON_OBJECT
EFIAPI
JsonValueGetObject (
  IN    EDKII_JSON_VALUE    Json
  )
{
  if (Json == NULL || !JsonValueIsObject (Json)) {
    return NULL;
  }

  return (EDKII_JSON_OBJECT) Json;
}

/**
  The function is used to retrieve the associated Ascii string in a string type JSON value.

  Any changes to the returned string will impact the original JSON value.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated Ascii string in JSON value or NULL.

**/
CHAR8*
EFIAPI
JsonValueGetAsciiString (
  IN    EDKII_JSON_VALUE    Json
  )
{
  CHAR8          *AsciiStr;
  UINTN          Index;

  AsciiStr = json_string_value ((json_t *) Json);
  if (AsciiStr == NULL) {
    return NULL;
  }

  Index = 0;
  while (*(AsciiStr + Index) != '\0') {
    if (((*(AsciiStr + Index)) & 0x80) != 0x00) {
      return NULL;
    }

    Index ++;
  }

  return AsciiStr;
}

/**
  The function is used to retrieve the associated Unicode string in a string type JSON value.

  Caller can do any changes to the returned string without any impact to the original JSON
  value, and caller needs to free the returned string.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated Unicode string in JSON value or NULL.

**/
CHAR16*
EFIAPI
JsonValueGetUnicodeString (
  IN    EDKII_JSON_VALUE    Json
  )
{
  EFI_STATUS     Status;
  CONST CHAR8    *Utf8Str;
  CHAR16         *Ucs2Str;

  Utf8Str = json_string_value ((json_t *) Json);
  if (Utf8Str == NULL) {
    return NULL;
  }

  Status = UTF8StrToUCS2 (Utf8Str, &Ucs2Str);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return Ucs2Str;
}

/**
  The function is used to retrieve the associated integer in a number type JSON value.

  The input JSON value should not be NULL or contain no JSON number, otherwise it will
  ASSERT() and return 0.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated number in JSON value.

**/
INT64
EFIAPI
JsonValueGetNumber (
  IN    EDKII_JSON_VALUE    Json
  )
{
  ASSERT (Json != NULL && JsonValueIsNumber (Json));
  if (Json == NULL || !JsonValueIsNumber (Json)) {
    return 0;
  }

  return json_integer_value ((json_t *) Json);
}

/**
  The function is used to retrieve the associated boolean in a boolean type JSON value.

  The input JSON value should not be NULL or contain no JSON boolean, otherwise it will
  ASSERT() and return FALSE.

  @param[in]   Json             The provided JSON value.

  @retval      Return the associated value of JSON boolean.

**/
BOOLEAN
EFIAPI
JsonValueGetBoolean (
  IN    EDKII_JSON_VALUE    Json
  )
{
  ASSERT (Json != NULL && JsonValueIsBoolean (Json));
  if (Json == NULL || !JsonValueIsBoolean (Json)) {
    return FALSE;
  }

  return json_is_true ((json_t *) Json);
}

/**
  The function is used to get the number of elements in a JSON object, or 0 if it is NULL or
  not a JSON object.

  @param[in]   JsonObject              The provided JSON object.

  @retval      Return the number of elements in this JSON object or 0.

**/
UINTN
EFIAPI
JsonObjectSize (
  IN    EDKII_JSON_OBJECT    JsonObject
  )
{
  return json_object_size ((json_t *) JsonObject);
}

/**
  The function is used to enumerate all keys in a JSON object.

  Caller should be responsible to free the returned key array refference. But contained keys
  are read only and must not be modified or freed.

  @param[in]   JsonObj                The provided JSON object for enumeration.
  @param[out]  KeyCount               The count of keys in this JSON object.

  @retval      Return an array of the enumerated keys in this JSON object or NULL.

**/
CHAR8**
JsonObjectGetKeys (
  IN    EDKII_JSON_OBJECT    JsonObj,
  OUT   UINTN                *KeyCount
  )
{

  UINTN               Index;
  CONST CHAR8         **KeyArray;
  CONST CHAR8         *Key;
  EDKII_JSON_VALUE    Value;

  if (JsonObj == NULL || KeyCount == NULL) {
    return NULL;
  }

  Index = 0;
  json_object_foreach(JsonObj, Key, Value) {
    Index ++;
  }
  if (Index == 0) {
    return NULL;
  }

  *KeyCount = Index;
  KeyArray = (CHAR8 **) AllocateZeroPool (*KeyCount * sizeof (CHAR8 *));
  if (KeyArray == NULL) {
    return NULL;
  }

  Key   = NULL;
  Value = NULL;
  Index = 0;
  json_object_foreach((json_t *) JsonObj, Key, Value) {
    KeyArray[Index] = Key;
    Index ++;
  }

  return KeyArray;
}

/**
  The function is used to get a JSON value corresponding to the input key from a JSON object.
  
  It only returns a reference to this value and any changes on this value will impact the
  original JSON object. If that is not expected, please call JsonValueClone() to clone it to
  use.
  
  Input key must be a valid NULL terminated UTF8 encoded string. NULL will be returned when 
  Key-Value is not found in this JSON object.

  @param[in]   JsonObj           The provided JSON object.
  @param[in]   Key               The key of the JSON value to be retrieved.

  @retval      Return the corresponding JSON value to key, or NULL on error.

**/
EDKII_JSON_VALUE
EFIAPI
JsonObjectGetValue (
  IN    EDKII_JSON_OBJECT    JsonObj,
  IN    CHAR8                *Key
  )
{
  return (EDKII_JSON_VALUE) json_object_get ((json_t *) JsonObj, Key);
}

/**
  The function is used to set a JSON value corresponding to the input key from a JSON object,
  and the reference count of this value will be increased by 1.

  Input key must be a valid NULL terminated UTF8 encoded string. If there already is a value for
  this key, this key will be assigned to the new JSON value. The old JSON value will be removed
  from this object and thus its' reference count will be decreased by 1.

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   JsonObj                The provided JSON object.
  @param[in]   Key                    The key of the JSON value to be set.
  @param[in]   Json                   The JSON value to set to this JSON object mapped by key.

  @retval      EFI_ABORTED            Some error occur and operation aborted.
  @retval      EFI_SUCCESS            The JSON value has been set to this JSON object.

**/
EFI_STATUS
EFIAPI
JsonObjectSetValue (
  IN    EDKII_JSON_OBJECT    JsonObj,
  IN    CHAR8                *Key,
  IN    EDKII_JSON_VALUE     Json
  )
{
  if (json_object_set ((json_t *) JsonObj, Key, (json_t *) Json) != 0) {
    return EFI_ABORTED;
  } else {
    return EFI_SUCCESS;
  }
}

/**
  The function is used to get the number of elements in a JSON array, or 0 if it is NULL or
  not a JSON array.

  @param[in]   JsonArray              The provided JSON array.

  @retval      Return the number of elements in this JSON array or 0.

**/
UINTN
EFIAPI
JsonArrayCount (
  IN    EDKII_JSON_ARRAY    JsonArray
  )
{
  return json_array_size ((json_t *) JsonArray);
}

/**
  The function is used to return the JSON value in the array at position index. The valid range
  for this index is from 0 to the return value of JsonArrayCount() minus 1.

  It only returns a reference to this value and any changes on this value will impact the
  original JSON object. If that is not expected, please call JsonValueClone() to clone it to
  use.

  If this array is NULL or not a JSON array, or if index is out of range, NULL will be returned.

  @param[in]   JsonArray         The provided JSON Array.

  @retval      Return the JSON value located in the Index position or NULL.

**/
EDKII_JSON_VALUE
EFIAPI
JsonArrayGetValue (
  IN    EDKII_JSON_ARRAY    JsonArray,
  IN    UINTN               Index
  )
{
  return (EDKII_JSON_VALUE) json_array_get ((json_t *) JsonArray, Index);
}

/**
  The function is used to append a JSON value to the end of the JSON array, and grow the size of
  array by 1. The reference count of this value will be increased by 1.

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   JsonArray              The provided JSON object.
  @param[in]   Json                   The JSON value to append.

  @retval      EFI_ABORTED            Some error occur and operation aborted.
  @retval      EFI_SUCCESS            JSON value has been appended to the end of the JSON array.

**/
EFI_STATUS
EFIAPI
JsonArrayAppendValue (
  IN    EDKII_JSON_ARRAY    JsonArray,
  IN    EDKII_JSON_VALUE    Json
  )
{
  if (json_array_append ((json_t *) JsonArray, (json_t *) Json) != 0) {
    return EFI_ABORTED;
  } else {
    return EFI_SUCCESS;
  }
}

/**
  The function is used to remove a JSON value at position index, shifting the elements after index
  one position towards the start of the array. The reference count of this value will be decreased
  by 1.

  More details for reference count strategy can refer to the API description for JsonValueFree().

  @param[in]   JsonArray              The provided JSON array.
  @param[in]   Index                  The Index position before removement.

  @retval      EFI_ABORTED            Some error occur and operation aborted.
  @retval      EFI_SUCCESS            The JSON array has been removed at position index.

**/
EFI_STATUS
EFIAPI
JsonArrayRemoveValue (
  IN    EDKII_JSON_ARRAY    JsonArray,
  IN    UINTN               Index
  )
{
  if (json_array_remove ((json_t *) JsonArray, Index) != 0) {
    return EFI_ABORTED;
  } else {
    return EFI_SUCCESS;
  }
}
