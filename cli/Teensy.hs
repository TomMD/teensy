{-# LANGUAGE DeriveDataTypeable #-}
module Main where

import qualified Data.ByteString as B
import qualified Data.ByteString.Char8 as C
import System.C.IO
import Foreign.C.Error
import Control.Concurrent
import Control.Monad (when)
import System.Environment (getArgs)
import Data.Word

import Commands

main = do
	print rwSize
	args <- getArgs
	when (length args < 2)
	  (error $ "Usage:" ++
		"\t./Teensy <opt> <teensy #> [index]\n" ++ 
		"\t-e <teensy#> [index]  Erase index or all indexes\n" ++
		"\t-r <teensy#> <indexes>  Read all given indexes\n" ++
		"\t-w <teensy#> <index> <data>  Writes the remaining data\n" ++
		"\t-s <teensy#> stress test\n")
	let (mode:teensy:rest) = args
	srM <- openTeensy ("/dev/teensy" ++ teensy) [O_RDWR]

	case srM of
		(Left err) -> putStrLn $ "Could not open teensy " ++ teensy
		(Right sr) -> handleArgs sr mode rest

handleArgs :: Teensy -> String -> [String] -> IO ()
handleArgs sr mode rest = do
	case mode of
		"-e" -> case rest of
				[idx] -> ioctlTeensy sr (TEENSY_IOCERASE_IDX (read idx)) >> return ()
				_     -> ioctlTeensy sr TEENSY_IOCFLUSH >> return ()
		"-r" -> case rest of
				[]     -> return ()
				blobs ->  mapM_ (\b -> readTeensy sr (BlobIdx $ read b) >>= print) rest
		"-w" -> case rest of
				(blob:val:_) -> do
					let idx = BlobIdx (read blob)
					    dat = read val
					putStrLn $ "Index " ++ (show idx) ++ "\tdata " ++ (show dat)
					writeTeensy sr idx dat >> return ()
				_      -> return ()
		"-s" -> stressTest sr
		_    -> return ()

stressTest sr = do
	let nrBlobs = 256
	mapM_ fillBlob   [0..nrBlobs - 1]
	res <- mapM verifyBlob [0..nrBlobs - 1]
	print res
  where
   fillBlob b = writeTeensy sr (blobIdx b) b
   verifyBlob b = fmap (== b) (readTeensy sr (blobIdx b))
   blobIdx = BlobIdx . fromIntegral
