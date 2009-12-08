{-# LANGUAGE GeneralizedNewtypeDeriving, ForeignFunctionInterface #-}
module Commands
	( Teensy
	, BlobIndex(..)
	, rwSize
	, TeensyIOCTL(..)
	, Command(..)
	, openTeensy
	, closeTeensy
	, readTeensy
	, writeTeensy
	, ioctlTeensy
	) where

import Data.ByteString as B
import Data.ByteString.Internal (createAndTrim)
import Data.Word
import Foreign.C.Error
import Foreign.C.Types
import Foreign.Storable
import Foreign.Marshal.Utils
import Foreign.Ptr
import System.C.IO
import System.IO
import Data.Bits

data Teensy = Teensy Int
newtype BlobIndex = BlobIdx Word64  deriving (Eq, Ord, Show, Num)

data Command = CMD_STORE | CMD_READ | CMD_DELETE | CMD_NULL deriving (Eq, Ord, Show)

instance Enum Command where
	fromEnum CMD_STORE  = 0x00
	fromEnum CMD_READ   = 0x03
	fromEnum CMD_DELETE = 0x06
	fromEnum CMD_NULL   = 0xFF
	toEnum 0x00 = CMD_STORE
	toEnum 0x03 = CMD_READ
	toEnum 0x06 = CMD_DELETE
	toEnum _ = CMD_NULL

data RWData = RWData
	{ cmd_type	:: Command
	, index		:: Word64
	, dat		:: Word16
	} deriving (Eq, Ord, Show)

instance Storable RWData where
	sizeOf _ = sizeOf (undefined :: Word8) + sizeOf (undefined :: Word64) + sizeOf (undefined :: Word16)
	alignment _ = max (alignment (undefined :: Word32)) (alignment (undefined :: Word64))
	peek ptr = do
		cmd <- peek (castPtr ptr) :: IO Word8
		idx <- peek (castPtr $ plusPtr ptr (sizeOf (undefined :: Word8)))
		dat  <- peek (castPtr $ plusPtr ptr (sizeOf idx + sizeOf (undefined :: Word8)))
		return $ RWData (toEnum $ fromIntegral cmd) idx dat
	poke ptr (RWData cmd idx p) = do
		poke (castPtr ptr) (fromIntegral (fromEnum cmd) :: Word8)
		poke (castPtr $ plusPtr ptr (sizeOf (undefined :: Word8))) idx
		poke (castPtr $ plusPtr ptr (sizeOf idx + sizeOf (undefined :: Word8))) p

rwSize = sizeOf (undefined :: RWData)

-- FIXME this is fragile, need to find a good way to import constants via c2hs
teensyMagic = fromEnum 't' `shiftL` 8

data TeensyIOCTL = TEENSY_IOCFLUSH | TEENSY_IOCERASE_IDX CULong
	deriving (Eq, Ord, Show)

instance Enum TeensyIOCTL where
	fromEnum TEENSY_IOCFLUSH = teensyMagic .|. 1
	fromEnum (TEENSY_IOCERASE_IDX _) = teensyMagic .|. 2
	toEnum x = case (x - teensyMagic) of
			1 -> TEENSY_IOCFLUSH
			2 -> TEENSY_IOCERASE_IDX undefined

foreign import ccall unsafe "ioctl"
	c_ioctl		:: Int -> Int -> CULong -> IO Int

ioctlTeensy :: Teensy -> TeensyIOCTL -> IO Int
ioctlTeensy (Teensy d) cmd =
	case cmd of
		TEENSY_IOCFLUSH ->
			c_ioctl d (fromEnum cmd) 0
		TEENSY_IOCERASE_IDX x ->
			c_ioctl d (fromEnum cmd) x

openTeensy :: FilePath -> [OFlag] -> IO (Either Errno Teensy)
openTeensy path flgs = do
	res <- copen path flgs
	if res == (-1) then fmap Left getErrno else return $ Right (Teensy res)

closeTeensy :: Teensy -> IO Bool
closeTeensy (Teensy i) = cclose i

readTeensy :: Teensy -> BlobIndex -> IO Word16
readTeensy sr@(Teensy fd) (BlobIdx idx) = do
	let rw = RWData CMD_READ idx 0
	with rw $ \rwPtr -> do
	res <- cread fd (castPtr rwPtr) rwSize
	resStruct <- peek rwPtr
	return (dat resStruct)

writeTeensy :: Teensy -> BlobIndex -> Word16 -> IO Int
writeTeensy (Teensy fd) (BlobIdx idx) dat = do
	let rw = RWData CMD_STORE idx dat
	with rw $ \rwPtr -> do
		cwrite fd (castPtr rwPtr) rwSize
