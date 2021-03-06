//! SkStream and relatives.
//! This implementation covers the minimal subset to interface with Rust streams.
//!
//! Bindings that wrap functions that use Skia stream types, _must_ use Rust streams instead.

use crate::prelude::*;
use crate::Data;
use skia_bindings as sb;
use skia_bindings::{SkDynamicMemoryWStream, SkMemoryStream, SkStream, SkStreamAsset, SkWStream};
use std::marker::PhantomData;
use std::ptr;

/// Trait representing an Skia allocated Stream type with a base class of SkStream.
#[repr(transparent)]
pub struct Stream<N: NativeStreamBase>(ptr::NonNull<N>);
unsafe impl<N: NativeStreamBase> Send for Stream<N> {}

pub trait NativeStreamBase {
    fn as_stream_mut(&mut self) -> &mut SkStream;
}

impl<T: NativeStreamBase> Drop for Stream<T> {
    fn drop(&mut self) {
        unsafe {
            sb::C_SkStream_delete(self.0.as_ptr() as *mut _);
        }
    }
}

impl<N: NativeStreamBase> Stream<N> {
    pub fn from_ptr(ptr: *mut N) -> Stream<N> {
        Stream(ptr::NonNull::new(ptr).unwrap())
    }
}

pub type StreamAsset = Stream<SkStreamAsset>;
impl NativeBase<SkStream> for SkStreamAsset {}

impl NativeStreamBase for SkStreamAsset {
    fn as_stream_mut(&mut self) -> &mut SkStream {
        self.base_mut()
    }
}

impl NativeAccess<SkStreamAsset> for Stream<SkStreamAsset> {
    fn native(&self) -> &SkStreamAsset {
        unsafe { self.0.as_ref() }
    }
    fn native_mut(&mut self) -> &mut SkStreamAsset {
        unsafe { self.0.as_mut() }
    }
}

#[repr(C)]
pub struct MemoryStream<'a> {
    native: ptr::NonNull<SkMemoryStream>,
    pd: PhantomData<&'a ()>,
}
unsafe impl Send for MemoryStream<'_> {}
impl NativeBase<SkStream> for SkMemoryStream {}

impl NativeStreamBase for SkMemoryStream {
    fn as_stream_mut(&mut self) -> &mut SkStream {
        self.base_mut()
    }
}

impl NativeAccess<SkMemoryStream> for MemoryStream<'_> {
    fn native(&self) -> &SkMemoryStream {
        unsafe { self.native.as_ref() }
    }
    fn native_mut(&mut self) -> &mut SkMemoryStream {
        unsafe { self.native.as_mut() }
    }
}

impl MemoryStream<'_> {
    // Create a stream asset that refers the bytes provided.
    pub fn from_bytes(bytes: &[u8]) -> MemoryStream {
        let ptr = unsafe { sb::C_SkMemoryStream_MakeDirect(bytes.as_ptr() as _, bytes.len()) };

        MemoryStream {
            native: ptr::NonNull::new(ptr).unwrap(),
            pd: PhantomData,
        }
    }
}

pub type DynamicMemoryWStream = Handle<SkDynamicMemoryWStream>;

impl NativeBase<SkWStream> for SkDynamicMemoryWStream {}

impl NativeDrop for SkDynamicMemoryWStream {
    fn drop(&mut self) {
        unsafe {
            sb::C_SkWStream_destruct(self.base_mut());
        }
    }
}

impl Handle<SkDynamicMemoryWStream> {
    pub fn new() -> Self {
        Self::construct(|dmws| unsafe { sb::C_SkDynamicMemoryWStream_Construct(dmws) })
    }

    pub fn from_bytes(bytes: &[u8]) -> Self {
        let mut stream = Self::new();
        stream.write(bytes);
        stream
    }

    pub fn write(&mut self, bytes: &[u8]) -> bool {
        unsafe {
            sb::C_SkWStream_write(
                self.native_mut().base_mut(),
                bytes.as_ptr() as _,
                bytes.len(),
            )
        }
    }

    pub fn detach_as_data(&mut self) -> Data {
        Data::from_ptr(unsafe { sb::C_SkDynamicMemoryWStream_detachAsData(self.native_mut()) })
            .unwrap()
    }

    pub fn detach_as_stream(&mut self) -> StreamAsset {
        StreamAsset::from_ptr(unsafe {
            sb::C_SkDynamicMemoryWStream_detachAsStream(self.native_mut())
        })
    }
}

#[test]
fn detaching_empty_dynamic_memory_w_stream_leads_to_non_null_data() {
    let mut stream = DynamicMemoryWStream::new();
    let data = stream.detach_as_data();
    assert_eq!(0, data.size())
}

#[test]
fn memory_stream_from_bytes() {
    let stream = MemoryStream::from_bytes(&[1, 2, 3]);
    drop(stream);
}
