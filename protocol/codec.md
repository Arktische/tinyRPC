## codec
codec design docs.

We divide serialization in Remote Procedure Call into 2 stage:
1. encode/decode for language built-in structure
2. serialize/deserialize it into/from stream with a specific binary format.

For example, in parsing RPC request scenario:
We have a binary byte array(also called stream), marked as `char []` for convinience.
First we have to decide which method it would call. So we need a field to identify a method Uniquely.
So the binary format might look like this:
```c++
struct Header{
    int header_len
    int method_name_len;
    char method_identifier[];
};

struct Data {
    type0 field;
    type1 field1;
    type2 field2;
};

struct Message {
    Header;
    Data;
    int32_t checksum; // extra checksum
};
```
But wait, there might be variable-length field in `Data` block as following shows:
```c++
struct GetStudentInfoRequest {
    int64_t user_id;
    vector<int> class_id;
};
```
So we need to add an extra length field to split these fields correctly in raw stream.