// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

import "forge-std/Script.sol";

contract MyScript is Script {
    function run() external {

        address instance = 0x831AE6eAB78a9D985fD3Ae6d871CA37F93937963;
        string memory password = "ethernaut0";
        vm.broadcast();
        (bool success, bytes memory data) = instance.call(abi.encodeWithSignature("authenticate(string)", password));
        require(success, "call failed");
    }
}
